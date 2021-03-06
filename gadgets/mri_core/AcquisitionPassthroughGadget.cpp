#include "GadgetIsmrmrdReadWrite.h"
#include "AcquisitionPassthroughGadget.h"
#include "Gadgetron.h"
namespace Gadgetron{
int AcquisitionPassthroughGadget
::process(GadgetContainerMessage<ISMRMRD::AcquisitionHeader>* m1,
	  GadgetContainerMessage< hoNDArray< std::complex<float> > >* m2)
{
  //It is enough to put the first one, since they are linked
  if (this->next()->putq(m1) == -1) {
    m1->release();
    ACE_ERROR_RETURN( (LM_ERROR,
		       ACE_TEXT("%p\n"),
		       ACE_TEXT("AcquisitionPassthroughGadget::process, passing data on to next gadget")),
		      -1);
  }

  return 0;
}
GADGET_FACTORY_DECLARE(AcquisitionPassthroughGadget)
}


