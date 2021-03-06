#include "GadgetIsmrmrdReadWrite.h"
#include "ImageFinishAttribGadget.h"

namespace Gadgetron
{
    template <typename T>
    int ImageFinishAttribGadget<T>::process(GadgetContainerMessage<ISMRMRD::ImageHeader>* m1, GadgetContainerMessage< hoNDArray< T > >* m2, GadgetContainerMessage<ISMRMRD::MetaContainer>* m3)
    {
        if (!this->controller_)
        {
            ACE_DEBUG( (LM_DEBUG, 
                ACE_TEXT("Cannot return result to controller, no controller set")) );
            return -1;
        }

        GadgetContainerMessage<GadgetMessageIdentifier>* mb = new GadgetContainerMessage<GadgetMessageIdentifier>();

        switch (sizeof(T))
        {
        case 2: //Unsigned short
            mb->getObjectPtr()->id = GADGET_MESSAGE_ISMRMRD_IMAGEWITHATTRIB_REAL_USHORT;
            break;
        case 4: //Float
            mb->getObjectPtr()->id = GADGET_MESSAGE_ISMRMRD_IMAGEWITHATTRIB_REAL_FLOAT;
            break;
        case 8: //Complex float
            mb->getObjectPtr()->id = GADGET_MESSAGE_ISMRMRD_IMAGEWITHATTRIB_CPLX_FLOAT;
            break;
        default:
            GADGET_DEBUG2("Wrong data size detected: %d\n", sizeof(T));
            mb->release();
            m1->release();
            return GADGET_FAIL;
        }

        mb->cont(m1);

        int ret =  this->controller_->output_ready(mb);

        if ( (ret < 0) )
        {
            GADGET_DEBUG1("Failed to return massage to controller\n");
            return GADGET_FAIL;
        }

        return GADGET_OK;
    }

    //Declare factories for the various template instances
    GADGET_FACTORY_DECLARE(ImageFinishAttribGadgetFLOAT)
    GADGET_FACTORY_DECLARE(ImageFinishAttribGadgetUSHORT)
    GADGET_FACTORY_DECLARE(ImageFinishAttribGadgetCPLX)
}
