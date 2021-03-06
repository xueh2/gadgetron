#include "NoiseAdjustGadget.h"
#include "Gadgetron.h"
#include "hoArmadillo.h"
#include "hoNDArray_elemwise.h"
#include "GadgetronCommon.h"
#include "hoMatrix.h"
#include "hoNDArray_linalg.h"
#include "hoNDArray_elemwise.h"

#ifdef USE_OMP
#include "omp.h"
#endif // USE_OMP

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#endif // _WIN32

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

namespace Gadgetron{

  NoiseAdjustGadget::NoiseAdjustGadget()
    : noise_decorrelation_calculated_(false)
    , number_of_noise_samples_(0)
    , number_of_noise_samples_per_acquisition_(0)
    , noise_bw_scale_factor_(1.0f)
    , noise_dwell_time_us_(-1.0f)
    , noiseCovarianceLoaded_(false)
  {
    noise_dependency_prefix_ = "GadgetronNoiseCovarianceMatrix";
    measurement_id_.clear();
    measurement_id_of_noise_dependency_.clear();
    noise_dwell_time_us_preset_ = 0.0;
    perform_noise_adjust_ = true;
  }

  NoiseAdjustGadget::~NoiseAdjustGadget()
  {

  }

  int NoiseAdjustGadget::process_config(ACE_Message_Block* mb)
  {

    boost::shared_ptr<std::string> str = this->get_string_value("workingDirectory");

    if ( !str->empty() ) {
      noise_dependency_folder_ = *str;
    }
    else {
#ifdef _WIN32
      noise_dependency_folder_ = std::string("c:\\temp\\gadgetron\\");
#else
      noise_dependency_folder_ =  std::string("/tmp/gadgetron/");
#endif // _WIN32
    }

    GADGET_DEBUG2("Folder to store noise dependencies is %s\n", noise_dependency_folder_.c_str());

    str = this->get_string_value("noise_dependency_prefix");
    if ( !str->empty() ) noise_dependency_prefix_ = *str;

    perform_noise_adjust_ = this->get_string_value("perform_noise_adjust")->size() ? this->get_bool_value("perform_noise_adjust") : true;
    GADGET_DEBUG2("NoiseAdjustGadget::perform_noise_adjust_ is %d\n", perform_noise_adjust_);

    noise_dwell_time_us_preset_ = (float)this->get_double_value("noise_dwell_time_us_preset");

    ISMRMRD::deserialize(mb->rd_ptr(),current_ismrmrd_header_);
    
    if ( current_ismrmrd_header_.acquisitionSystemInformation ) {
      receiver_noise_bandwidth_ = (float)(current_ismrmrd_header_.acquisitionSystemInformation->relativeReceiverNoiseBandwidth ?
					  *current_ismrmrd_header_.acquisitionSystemInformation->relativeReceiverNoiseBandwidth : 0.793f);
      
      GADGET_DEBUG2("receiver_noise_bandwidth_ is %f\n", receiver_noise_bandwidth_);
    }

    // find the measurementID of this scan
    if ( current_ismrmrd_header_.measurementInformation )
      {
	if ( current_ismrmrd_header_.measurementInformation->measurementID )
	  {
	    measurement_id_ = *current_ismrmrd_header_.measurementInformation->measurementID;
	    GADGET_DEBUG2("Measurement ID is %s\n", measurement_id_.c_str());
	  }

	// find the noise depencies if any
	if ( current_ismrmrd_header_.measurementInformation->measurementDependency.size() > 0 )
	  {
	    measurement_id_of_noise_dependency_.clear();

	    std::vector<ISMRMRD::MeasurementDependency>::const_iterator iter = current_ismrmrd_header_.measurementInformation->measurementDependency.begin();
	    for ( ; iter!= current_ismrmrd_header_.measurementInformation->measurementDependency.end(); iter++ )
	      {
		std::string dependencyType = iter->dependencyType;
		std::string dependencyID = iter->measurementID;

		GADGET_DEBUG2("Found dependency measurement : %s with ID %s\n", dependencyType.c_str(), dependencyID.c_str());
            
		if ( dependencyType=="Noise" || dependencyType=="noise" ) {
		  measurement_id_of_noise_dependency_ = dependencyID;
		}
	      }
        
	    if ( !measurement_id_of_noise_dependency_.empty() ) {
	      GADGET_DEBUG2("Measurement ID of noise dependency is %s\n", measurement_id_of_noise_dependency_.c_str());
		  
	      full_name_stored_noise_dependency_ = this->generateNoiseDependencyFilename(generateMeasurementIdOfNoiseDependency(measurement_id_of_noise_dependency_));
	      GADGET_DEBUG2("Stored noise dependency is %s\n", full_name_stored_noise_dependency_.c_str());
		  
	      // try to load the precomputed noise prewhitener
	      if ( !this->loadNoiseCovariance() ) {
		GADGET_DEBUG2("Stored noise dependency is NOT found : %s\n", full_name_stored_noise_dependency_.c_str());
		noiseCovarianceLoaded_ = false;
		noise_dwell_time_us_ = -1;
		noise_covariance_matrixf_.clear();
	      } else {
		GADGET_DEBUG2("Stored noise dependency is found : %s\n", full_name_stored_noise_dependency_.c_str());
		GADGET_DEBUG2("Stored noise dwell time in us is %f\n", noise_dwell_time_us_);
		GADGET_DEBUG2("Stored noise channel number is %d\n", noise_covariance_matrixf_.get_size(0));
		
		if (noise_ismrmrd_header_.acquisitionSystemInformation) {
		  if (noise_ismrmrd_header_.acquisitionSystemInformation->coilLabel.size() != 
		      current_ismrmrd_header_.acquisitionSystemInformation->coilLabel.size()) {
		    GADGET_DEBUG1("Length of coil label arrays do not match");
		    return GADGET_FAIL;
		  }
		  
		  bool labels_match = true;
		  for (size_t l = 0; l < noise_ismrmrd_header_.acquisitionSystemInformation->coilLabel.size(); l++) {
		    if (noise_ismrmrd_header_.acquisitionSystemInformation->coilLabel[l].coilNumber != 
			current_ismrmrd_header_.acquisitionSystemInformation->coilLabel[l].coilNumber) {
		      labels_match = false; break;
		    }
		    if (noise_ismrmrd_header_.acquisitionSystemInformation->coilLabel[l].coilName != 
			current_ismrmrd_header_.acquisitionSystemInformation->coilLabel[l].coilName) {
		      labels_match = false; break;
		    }
		  }
		  if (!labels_match) {
		    GADGET_DEBUG1("Noise and measurement coil labels don't match\n");
		    return GADGET_FAIL;
		  }
		} else if (current_ismrmrd_header_.acquisitionSystemInformation) {
		  GADGET_DEBUG1("Noise ismrmrd header does not have acquisition system information but current header does\n");
		  return GADGET_FAIL;
		}

		noiseCovarianceLoaded_ = true;
		number_of_noise_samples_ = 1; //When we load the matrix, it is already scaled.
	      }
	    }
	  }
      }


    //Let's figure out if some channels are "scale_only"
    boost::shared_ptr<std::string> uncomb_str = this->get_string_value("scale_only_channels_by_name");
    std::vector<std::string> uncomb;
    if (uncomb_str->size()) {
      GADGET_DEBUG2("SCALE ONLY: %s\n",  uncomb_str->c_str());
      boost::split(uncomb, *uncomb_str, boost::is_any_of(","));
      for (unsigned int i = 0; i < uncomb.size(); i++) {
	std::string ch = boost::algorithm::trim_copy(uncomb[i]);
	if (current_ismrmrd_header_.acquisitionSystemInformation) {
	  for (size_t i = 0; i < current_ismrmrd_header_.acquisitionSystemInformation->coilLabel.size(); i++) {
	    if (ch == current_ismrmrd_header_.acquisitionSystemInformation->coilLabel[i].coilName) {
	      scale_only_channels_.push_back(i);//This assumes that the channels are sorted in the header
	      break;
	    }
	  }
	}
      }
    }

#ifdef USE_OMP
    omp_set_num_threads(1);
#endif // USE_OMP

    return GADGET_OK;
  }

  std::string NoiseAdjustGadget::generateMeasurementIdOfNoiseDependency(const std::string& noise_id)
  {
    // find the scan prefix
    std::string measurementStr = measurement_id_;
    size_t ind  = measurement_id_.find_last_of ("_");
    if ( ind != std::string::npos ) {
      measurementStr = measurement_id_.substr(0, ind);
      measurementStr.append("_");
      measurementStr.append(noise_id);
    }
   
    return measurementStr;
  }

  std::string NoiseAdjustGadget::generateNoiseDependencyFilename(const std::string& measurement_id)
  {
    std::string full_name_stored_noise_dependency;

    full_name_stored_noise_dependency = noise_dependency_folder_;
    full_name_stored_noise_dependency.append("/");
    full_name_stored_noise_dependency.append(noise_dependency_prefix_);
    full_name_stored_noise_dependency.append("_");
    full_name_stored_noise_dependency.append(measurement_id);

    return full_name_stored_noise_dependency;
  }

  bool NoiseAdjustGadget::loadNoiseCovariance()
  {
    std::ifstream infile;
    infile.open (full_name_stored_noise_dependency_.c_str(), std::ios::in|std::ios::binary);

    if (infile.good()) {
      //Read the XML header of the noise scan
      uint32_t xml_length;
      infile.read( reinterpret_cast<char*>(&xml_length), 4);
      std::string xml_str(xml_length,'\0');
      infile.read(const_cast<char*>(xml_str.c_str()), xml_length);
      ISMRMRD::deserialize(xml_str.c_str(), noise_ismrmrd_header_);
	
      infile.read( reinterpret_cast<char*>(&noise_dwell_time_us_), sizeof(float));

      size_t len;
      infile.read( reinterpret_cast<char*>(&len), sizeof(size_t));

      char* buf = new char[len];
      if ( buf == NULL ) return false;

      infile.read(buf, len);

      if ( !noise_covariance_matrixf_.deserialize(buf, len) )
	{
	  delete [] buf;
	  return false;
	}

      delete [] buf;
      infile.close();
    } else {
      GADGET_DEBUG1("Noise prewhitener file is not found. Proceeding without stored noise\n");
      return false;
    }

    return true;
  }

  bool NoiseAdjustGadget::saveNoiseCovariance()
  {
    char* buf = NULL;
    size_t len(0);
    
    //Do we have any noise?
    if (noise_covariance_matrixf_.get_number_of_elements() == 0) {
      return true;
    }

    //Scale the covariance matrix before saving
    hoNDArray< std::complex<float> > covf(noise_covariance_matrixf_);

    if (number_of_noise_samples_ > 1) {
      covf *= std::complex<float>(1.0/(float)(number_of_noise_samples_-1),0.0);
    }

    if ( !covf.serialize(buf, len) ) {
      GADGET_DEBUG1("Noise covariance serialization failed ...\n");
      return false;
    }

    std::stringstream xml_ss;
    ISMRMRD::serialize(current_ismrmrd_header_, xml_ss);
    std::string xml_str = xml_ss.str();
    uint32_t xml_length = static_cast<uint32_t>(xml_str.size());

    std::ofstream outfile;
    std::string filename  = this->generateNoiseDependencyFilename(measurement_id_);
    outfile.open (filename.c_str(), std::ios::out|std::ios::binary);

    if (outfile.good())
      {
	GADGET_DEBUG2("write out the noise dependency file : %s\n", filename.c_str());
	outfile.write( reinterpret_cast<char*>(&xml_length), 4);
	outfile.write( xml_str.c_str(), xml_length );
	outfile.write( reinterpret_cast<char*>(&noise_dwell_time_us_), sizeof(float));
	outfile.write( reinterpret_cast<char*>(&len), sizeof(size_t));
	outfile.write(buf, len);
	outfile.close();

	// set the permission for the noise file to be rewritable
#ifndef _WIN32
	int res = chmod(filename.c_str(), S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
	if ( res != 0 ) {
	  GADGET_DEBUG1("Changing noise prewhitener file permission failed ...\n");
	}
#endif // _WIN32
      } else {
      delete [] buf;
      GADGET_ERROR_MSG("Noise prewhitener file is not good for writing");
      return false;
    }

    delete [] buf;
    return true;
  }

  void NoiseAdjustGadget::computeNoisePrewhitener()
  {
    GADGET_DEBUG2("Noise dwell time: %f\n", noise_dwell_time_us_);
    GADGET_DEBUG2("receiver_noise_bandwidth: %f\n", receiver_noise_bandwidth_);
    
    if (!noise_decorrelation_calculated_) {
      
      if (number_of_noise_samples_ > 0 ) {
	GADGET_DEBUG1("Calculating noise decorrelation\n");
	
	noise_prewhitener_matrixf_ = noise_covariance_matrixf_;
	
	//Mask out scale  only channels
	size_t c = noise_prewhitener_matrixf_.get_size(0);
	std::complex<float>* dptr = noise_prewhitener_matrixf_.get_data_ptr(); 
	for (unsigned int ch = 0; ch < scale_only_channels_.size(); ch++) {
	  for (size_t i = 0; i <  c; i++) {
	    for (size_t j = 0; j < c; j++) {
	      if ((i == scale_only_channels_[ch] || (j == scale_only_channels_[ch])) && (i != j)) { //zero if scale only and not on diagonal
		dptr[i*c+j] = std::complex<float>(0.0,0.0);
	      }
	    }
	  }
	}

	//Cholesky and invert lower triangular
	arma::cx_fmat noise_covf = as_arma_matrix(&noise_prewhitener_matrixf_);      
	noise_covf = arma::inv(arma::trimatu(arma::chol(noise_covf)));
      
	noise_decorrelation_calculated_ = true;
      } else {
	noise_decorrelation_calculated_ = false;
      }
    }
  }

  int NoiseAdjustGadget::process(GadgetContainerMessage<ISMRMRD::AcquisitionHeader>* m1, GadgetContainerMessage< hoNDArray< std::complex<float> > >* m2)
  {
    bool is_noise = m1->getObjectPtr()->isFlagSet(ISMRMRD::ISMRMRD_ACQ_IS_NOISE_MEASUREMENT);
    unsigned int channels = m1->getObjectPtr()->active_channels;
    unsigned int samples = m1->getObjectPtr()->number_of_samples;

    //TODO: Remove this
    if ( measurement_id_.empty() ) {
      unsigned int muid = m1->getObjectPtr()->measurement_uid;
      std::ostringstream ostr;
      ostr << muid;
      measurement_id_ = ostr.str();
    }

    if ( is_noise ) {
      if (noiseCovarianceLoaded_) {
	m1->release(); //Do not accumulate noise when we have a loaded noise covariance
	return GADGET_OK;
      }
      
      // this noise can be from a noise scan or it can be from the built-in noise
      if ( number_of_noise_samples_per_acquisition_ == 0 ) {
	number_of_noise_samples_per_acquisition_ = samples;
      }

      if ( noise_dwell_time_us_ < 0 ) {
	if (noise_dwell_time_us_preset_ > 0.0) {
	  noise_dwell_time_us_ = noise_dwell_time_us_preset_;
	} else {
	  noise_dwell_time_us_ = m1->getObjectPtr()->sample_time_us;
	}
      }

      //If noise covariance matrix is not allocated
      if (noise_covariance_matrixf_.get_number_of_elements() != channels*channels) {
	std::vector<size_t> dims(2, channels);
	try {
	  noise_covariance_matrixf_.create(&dims);
	  noise_covariance_matrixf_once_.create(&dims);
	} catch (std::runtime_error& err) {
	  GADGET_DEBUG_EXCEPTION(err, "Unable to allocate storage for noise covariance matrix\n" );
	  return GADGET_FAIL;
	}

	Gadgetron::clear(noise_covariance_matrixf_);
	Gadgetron::clear(noise_covariance_matrixf_once_);
	number_of_noise_samples_ = 0;
      }

      std::complex<float>* cc_ptr = noise_covariance_matrixf_.get_data_ptr();
      std::complex<float>* data_ptr = m2->getObjectPtr()->get_data_ptr();
      
      hoNDArray< std::complex<float> > readout(*m2->getObjectPtr());
      gemm(noise_covariance_matrixf_once_, readout, true, *m2->getObjectPtr(), false);
      Gadgetron::add(noise_covariance_matrixf_once_, noise_covariance_matrixf_, noise_covariance_matrixf_);
      
      number_of_noise_samples_ += samples;
      m1->release();
      return GADGET_OK;
    }


    //We should only reach this code if this data is not noise.
    if ( perform_noise_adjust_ ) {
      //Calculate the prewhitener if it has not been done
      if (!noise_decorrelation_calculated_ && (number_of_noise_samples_ > 0)) {
	if (number_of_noise_samples_ > 1) {
	  //Scale
	  noise_covariance_matrixf_ *= std::complex<float>(1.0/(float)(number_of_noise_samples_-1));
	  number_of_noise_samples_ = 1; //Scaling has been done
	}
	computeNoisePrewhitener();
	acquisition_dwell_time_us_ = m1->getObjectPtr()->sample_time_us;
	if ((noise_dwell_time_us_ == 0.0f) || (acquisition_dwell_time_us_ == 0.0f)) {
	  noise_bw_scale_factor_ = 1.0f;
	} else {
	  noise_bw_scale_factor_ = (float)std::sqrt(2.0*acquisition_dwell_time_us_/noise_dwell_time_us_*receiver_noise_bandwidth_);
	}

	noise_prewhitener_matrixf_ *= std::complex<float>(noise_bw_scale_factor_,0.0);

	GADGET_DEBUG2("Noise dwell time: %f\n", noise_dwell_time_us_);
	GADGET_DEBUG2("Acquisition dwell time: %f\n", acquisition_dwell_time_us_);
	GADGET_DEBUG2("receiver_noise_bandwidth: %f\n", receiver_noise_bandwidth_);
	GADGET_DEBUG2("noise_bw_scale_factor: %f", noise_bw_scale_factor_);
      }

      if (noise_decorrelation_calculated_) {
	//Apply prewhitener
	hoNDArray<std::complex<float> > tmp(*m2->getObjectPtr());
	gemm(*m2->getObjectPtr(), tmp, noise_prewhitener_matrixf_);
      }
    }

    if (this->next()->putq(m1) == -1) {
      GADGET_DEBUG1("Error passing on data to next gadget\n");
      return GADGET_FAIL;
    }
    
    return GADGET_OK;

  }

  int NoiseAdjustGadget::close(unsigned long flags)
  {
    if ( BaseClass::close(flags) != GADGET_OK ) return GADGET_FAIL;

    static bool saved = false; //Static variable to make sure we only save it once

    if ( !noiseCovarianceLoaded_  && !saved ){
      saveNoiseCovariance();
      saved = true;
    }  

    return GADGET_OK;
  }

  GADGET_FACTORY_DECLARE(NoiseAdjustGadget)

} // namespace Gadgetron
