#include "cuNDArray_blas.h"
#include "complext.h"
#include "GadgetronCuException.h"
#include "cudaDeviceManager.h"

#include <cublas_v2.h>

namespace Gadgetron{

#define CUBLAS_CALL(fun) {cublasStatus_t err = fun; if (err != CUBLAS_STATUS_SUCCESS) {throw cuda_error(getCublasErrorString(err));}}

  //NRM2
  //

  template<> EXPORTGPUCORE cublasStatus_t cublas_nrm2<float>(cublasHandle_t hndl, int n, const float*  x, int inc, float* res){
    return cublasSnrm2(hndl,n,x,inc,res);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_nrm2<double>(cublasHandle_t hndl, int n, const double*  x, int inc, double* res){
    return cublasDnrm2(hndl,n,x,inc,res);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_nrm2<float_complext>(cublasHandle_t hndl, int n, const float_complext*  x, int inc, float* res){
    return cublasScnrm2(hndl,n,(const cuComplex*)x,inc,res);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_nrm2<double_complext>(cublasHandle_t hndl, int n, const double_complext*  x, int inc, double* res){
    return cublasDznrm2(hndl,n,(const cuDoubleComplex*) x,inc,res);
  }

  //DOT
  //

  template<> EXPORTGPUCORE cublasStatus_t cublas_dot<float>(cublasHandle_t hndl, int n , const float* x , int incx, const  float* y , int incy, float* res, bool cc){
    return cublasSdot( hndl, n, x, incx, y, incy, res);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_dot<double>(cublasHandle_t hndl, int n , const double* x , int incx, const  double* y , int incy, double* res, bool cc){
    return cublasDdot( hndl, n, x, incx, y, incy, res);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_dot<float_complext>(cublasHandle_t hndl, int n , const float_complext* x ,
										int incx, const  float_complext* y , int incy, float_complext* res, bool cc){
    if(cc)
      return cublasCdotc( hndl, n, (const cuComplex*) x, incx, (const cuComplex*) y, incy, (cuComplex*) res);
    else
      return cublasCdotu( hndl, n, (const cuComplex*) x, incx, (const cuComplex*) y, incy, (cuComplex*) res);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_dot<double_complext>(cublasHandle_t hndl, int n , const double_complext* x ,
										 int incx, const  double_complext* y , int incy, double_complext* res, bool cc){
    if(cc)
      return cublasZdotc( hndl, n, (const cuDoubleComplex*) x, incx, (const cuDoubleComplex*) y, incy, (cuDoubleComplex*) res);
    else
      return cublasZdotu( hndl, n, (const cuDoubleComplex*) x, incx, (const cuDoubleComplex*) y, incy, (cuDoubleComplex*) res);
  }

  // AXPY
  //

  template<> EXPORTGPUCORE cublasStatus_t cublas_axpy<float>(cublasHandle_t hndl , int n , const float* a , const float* x , int incx ,  float* y , int incy){
    return cublasSaxpy(hndl,n,a,x,incx,y,incy);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_axpy<double>(cublasHandle_t hndl , int n , const double* a , const double* x , int incx ,  double* y , int incy){
    return cublasDaxpy(hndl,n,a,x,incx,y,incy);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_axpy<float_complext>(cublasHandle_t hndl , int n , const float_complext* a , const float_complext* x , int incx ,  float_complext* y , int incy){
    return cublasCaxpy(hndl,n,(const cuComplex*) a, (const cuComplex*) x,incx, (cuComplex*)y,incy);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_axpy<double_complext>(cublasHandle_t hndl , int n , const double_complext* a , const double_complext* x , int incx ,  double_complext* y , int incy){
    return cublasZaxpy(hndl,n,(const cuDoubleComplex*) a, (const cuDoubleComplex*) x,incx, (cuDoubleComplex*)y,incy);
  }

  //SUM
  //

  template<> EXPORTGPUCORE cublasStatus_t cublas_asum<float>(cublasHandle_t hndl, int n,const float *x, int incx, float *result){
    return cublasSasum(hndl,n,x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_asum<double>(cublasHandle_t hndl, int n,const double *x, int incx, double *result){
    return cublasDasum(hndl,n,x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_asum<float_complext>(cublasHandle_t hndl, int n,const float_complext *x, int incx, float *result){
    return cublasScasum(hndl,n,(const cuComplex*) x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_asum<double_complext>(cublasHandle_t hndl, int n,const double_complext *x, int incx, double *result){
    return cublasDzasum(hndl,n,(const cuDoubleComplex*) x,incx,result);
  }

  //AMIN
  //

  template<> EXPORTGPUCORE cublasStatus_t cublas_amin<float>(cublasHandle_t hndl, int n,const float *x, int incx, int *result){
    return cublasIsamin(hndl,n,x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_amin<double>(cublasHandle_t hndl, int n,const double *x, int incx, int *result){
    return cublasIdamin(hndl,n,x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_amin<float_complext>(cublasHandle_t hndl, int n,const float_complext *x, int incx, int *result){
    return cublasIcamin(hndl,n, (const cuComplex* ) x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_amin<double_complext>(cublasHandle_t hndl, int n,const double_complext *x, int incx, int *result){
    return cublasIzamin(hndl,n, (const cuDoubleComplex* ) x,incx,result);
  }

  //AMAX
  //

  template<> EXPORTGPUCORE cublasStatus_t cublas_amax<float>(cublasHandle_t hndl, int n,const float *x, int incx, int *result){
    return cublasIsamax(hndl,n,x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_amax<double>(cublasHandle_t hndl, int n,const double *x, int incx, int *result){
    return cublasIdamax(hndl,n,x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_amax<float_complext>(cublasHandle_t hndl, int n,const float_complext *x, int incx, int *result){
    return cublasIcamax(hndl,n, (const cuComplex* ) x,incx,result);
  }

  template<> EXPORTGPUCORE cublasStatus_t cublas_amax<double_complext>(cublasHandle_t hndl, int n,const double_complext *x, int incx, int *result){
    return cublasIzamax(hndl,n, (const cuDoubleComplex* ) x,incx,result);
  }

  template<class T> typename realType<T>::Type nrm2( cuNDArray<T> *arr )
  {
    if( arr == 0x0 )
      throw std::runtime_error("Gadgetron::nrm2(): Invalid input array");

    int device = cudaDeviceManager::Instance()->getCurrentDevice();
    typedef typename realType<T>::Type REAL;
    REAL ret;

    CUBLAS_CALL(cublas_nrm2<T>( cudaDeviceManager::Instance()->getHandle(device), arr->get_number_of_elements(),
				arr->get_data_ptr(), 1,
				&ret));
    return ret;
  }

  template<class T> T dot( cuNDArray<T> *arr1, cuNDArray<T> *arr2, bool cc )
  {
    if( arr1 == 0x0 || arr2 == 0x0 )
      throw std::runtime_error("Gadgetron::dot(): Invalid input array");
    
    if( arr1->get_number_of_elements() != arr2->get_number_of_elements() )
      throw std::runtime_error("Gadgetron::dot(): Array sizes mismatch");

    int device = cudaDeviceManager::Instance()->getCurrentDevice();
    T ret;

    CUBLAS_CALL(cublas_dot( cudaDeviceManager::Instance()->getHandle(device), arr1->get_number_of_elements(),
			    arr1->get_data_ptr(), 1,
			    arr2->get_data_ptr(), 1,
			    &ret, cc ));
    return ret;
  }

  template<class T> void axpy( T a, cuNDArray<T> *x, cuNDArray<T> *y )
  {
    if( x == 0x0 || y == 0x0 )
      throw std::runtime_error("Gadgetron::axpy(): Invalid input array");
    
    if( x->get_number_of_elements() != y->get_number_of_elements() )
      throw std::runtime_error("Gadgetron::axpy(): Array sizes mismatch");

    int device = cudaDeviceManager::Instance()->getCurrentDevice();
    CUBLAS_CALL(cublas_axpy(cudaDeviceManager::Instance()->getHandle(device), x->get_number_of_elements(),
			    &a, x->get_data_ptr(), 1,
			    y->get_data_ptr(), 1));
  }

  template<class T> void axpy( T a,  cuNDArray< complext<T> > *x, cuNDArray< complext<T> > *y )
  {
    axpy( complext<T>(a), x, y );
  }

  template<class T> typename realType<T>::Type asum(cuNDArray<T>* x)
  {
    if( x == 0x0 )
      throw std::runtime_error("Gadgetron::asum(): Invalid input array");
    
    int device = cudaDeviceManager::Instance()->getCurrentDevice();
    typename realType<T>::Type result;
    CUBLAS_CALL(cublas_asum(cudaDeviceManager::Instance()->getHandle(device),x->get_number_of_elements(),x->get_data_ptr(),1,&result));
    return result;
  }
  
  template<class T> int amin( cuNDArray<T>* x )
  {
    if( x == 0x0 )
      throw std::runtime_error("Gadgetron::amin(): Invalid input array");

    int device = cudaDeviceManager::Instance()->getCurrentDevice();
    int result;

    CUBLAS_CALL(cublas_amin(cudaDeviceManager::Instance()->getHandle(device),x->get_number_of_elements(),x->get_data_ptr(),1,&result));
    
    if( result > x->get_number_of_elements() ){
      throw std::runtime_error("Gadgetron::amin(): computed index is out of bounds");
    }
    
    return result-1;
  }
  
  template<class T> int amax(cuNDArray<T> *x )
  {
    if( x == 0x0 )
      throw std::runtime_error("Gadgetron::amax(): Invalid input array");

    int device = cudaDeviceManager::Instance()->getCurrentDevice();
    int result;

    CUBLAS_CALL(cublas_amax(cudaDeviceManager::Instance()->getHandle(device),x->get_number_of_elements(),x->get_data_ptr(),1,&result));

    if( result > x->get_number_of_elements() ){

      result = amax(x);
      
      if( result > x->get_number_of_elements() ){
	throw std::runtime_error("Gadgetron::amax(): computed index is out of bounds");
      }
      else{
	std::cout << std::endl << "WARNING: spooky inconsistent behaviour from Gadgetron::amax(cuNDArray*). Be aware" << std::endl;
      }
    }
    
    return result-1;
  }
  
  std::string getCublasErrorString(cublasStatus_t err)
  {
    switch (err){
    case CUBLAS_STATUS_NOT_INITIALIZED:
      return "NOT INITIALIZED";
    case CUBLAS_STATUS_ALLOC_FAILED:
      return "ALLOC FAILED";
    case CUBLAS_STATUS_INVALID_VALUE:
      return "INVALID VALUE";
    case CUBLAS_STATUS_ARCH_MISMATCH:
      return "ARCH MISMATCH";
    case CUBLAS_STATUS_MAPPING_ERROR:
      return "MAPPING ERROR";
    case CUBLAS_STATUS_EXECUTION_FAILED:
      return "EXECUTION FAILED";
    case CUBLAS_STATUS_INTERNAL_ERROR:
      return "INTERNAL ERROR";      
    case CUBLAS_STATUS_SUCCESS:
      return "SUCCES";
    default:
      return "UNKNOWN CUBLAS ERROR";
    }
  }
  
  //
  // Instantiation
  //
  
  template EXPORTGPUCORE float dot(cuNDArray<float>*,cuNDArray<float>*,bool);
  template EXPORTGPUCORE float nrm2(cuNDArray<float>*);
  template EXPORTGPUCORE void axpy(float,cuNDArray<float>*,cuNDArray<float>*);
  template EXPORTGPUCORE int amin(cuNDArray<float>*);
  template EXPORTGPUCORE int amax(cuNDArray<float>*);
  template EXPORTGPUCORE float asum(cuNDArray<float>*);

  template EXPORTGPUCORE double dot(cuNDArray<double>*,cuNDArray<double>*,bool);
  template EXPORTGPUCORE double nrm2(cuNDArray<double>*);
  template EXPORTGPUCORE void axpy(double,cuNDArray<double>*,cuNDArray<double>*);
  template EXPORTGPUCORE int amin(cuNDArray<double>*);
  template EXPORTGPUCORE int amax(cuNDArray<double>*);
  template EXPORTGPUCORE double asum(cuNDArray<double>*);

  template EXPORTGPUCORE float_complext dot(cuNDArray<float_complext>*,cuNDArray<float_complext>*,bool);
  template EXPORTGPUCORE float nrm2(cuNDArray<float_complext>*);
  template EXPORTGPUCORE void axpy(float_complext,cuNDArray<float_complext>*,cuNDArray<float_complext>*);
  template EXPORTGPUCORE void axpy(float,cuNDArray<float_complext>*,cuNDArray<float_complext>*);
  template EXPORTGPUCORE int amin(cuNDArray<float_complext>*);
  template EXPORTGPUCORE int amax(cuNDArray<float_complext>*);
  template EXPORTGPUCORE float asum(cuNDArray<float_complext>*);

  template EXPORTGPUCORE double_complext dot(cuNDArray<double_complext>*,cuNDArray<double_complext>*,bool);
  template EXPORTGPUCORE double nrm2(cuNDArray<double_complext>*);
  template EXPORTGPUCORE void axpy(double_complext,cuNDArray<double_complext>*,cuNDArray<double_complext>*);
  template EXPORTGPUCORE void axpy(double,cuNDArray<double_complext>*,cuNDArray<double_complext>*);
  template EXPORTGPUCORE int amin(cuNDArray<double_complext>*);
  template EXPORTGPUCORE int amax(cuNDArray<double_complext>*);
  template EXPORTGPUCORE double asum(cuNDArray<double_complext>*);
}
