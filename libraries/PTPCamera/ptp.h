#ifndef __PTP_H__
#define __PTP_H__

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <Max3421e.h>
#include <Usb.h>
#include "ptpconst.h"
#include "ptpmsgstr.h"
#include "ptpcallback.h"
#include "ptpdebug.h"

// Buffer size should NEVER be less than USB packet size!!!!!!!!!!!!!!!!!!!!!
#define PTP_MAX_RX_BUFFER_LEN   64
#define PTP_MAX_EV_BUFFER_LEN    8

typedef void (*PTPMAIN)();

class PTP
{
#ifdef PTP_HANDLE_RESPONSES
        static const char* stdResponse[];
#endif

        //static const char* dpValNames[];
        static const char* dtNames1[];
        static const char* dtNames2[];

        uint8_t                         devAddress;
        uint8_t                         epDataIn;
        uint8_t                         epDataOut;
        uint8_t                         epInterrupt;
        uint8_t                         numConf;

        MAX3421E                        Max;
        USB                                     Usb;

        uint16_t                        idTransaction;
        uint16_t                        idSession;

        PTPMAIN                         pfRunning;

protected:
        struct OperFlags
        {
                uint16_t        opParams        :       3;                      // 7    - maximum number of operation parameters
                uint16_t        rsParams        :       3;                      // 7    - maximum number of response parameters
                uint16_t        txOperation     :       1;                      // I->R operation if the flag is set
                uint16_t        dataStage       :       1;                      // operation has data stage if the flag is set
                uint16_t        typeOfVoid      :       2;                      // 0 - NULL, 1 - PTPReadParser/PTPDataSupplyer, 2 - WRITEPARSER, 3 - buffer pointer
                uint16_t        dataSize        :       6;                      // size of data buffer (64 bytes maximum)               
        };
        typedef void (*READPARSER)(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset);

        void ZerroMemory(uint8_t size, uint8_t *mem) { for (uint8_t i=0; i<size; i++) mem[i] = 0; };

        // waits for any event to occure
        // returns event on success or error code if timeout elapsed
        bool EventWait(uint8_t size, uint8_t *event_buf, uint16_t timeout);

        // returns true if event occured
        // the actual data is stored in a buffer pointed by buf
        bool CheckEvent(uint8_t size, uint8_t *buf);

        uint16_t Transaction(uint16_t opcode, OperFlags *flags, uint32_t *params, void *pVoid);

        void HaltOnError(const char* msg, uint16_t rcode) { Message(msg, rcode); while(1); };
        //void PrintDevicePropValue(PTPDevicePropDesc *prop);

#ifdef PTP_HANDLE_RESPONSES
        // outputs human readable response messages
        void HandleResponse(uint16_t rc);
#endif

public:
        PTP(uint8_t addr, uint8_t epin, uint8_t epout, uint8_t epint, uint8_t nconf, PTPMAIN pfunc);

        virtual uint16_t EventCheck(PTPReadParser *parser);

        void    Setup() { Max.powerOn(); };
        void    Task();

        //uint8_t GetUsbState() { return Usb.GetUsbState()      };

        // Simple PTP operation which has no data stage. Any number of uint32_t params can be supplied.
        uint16_t Operation(uint16_t opcode, uint8_t nparams = 0, uint32_t *params = NULL);

        uint16_t CaptureImage();

        uint16_t OpenSession();
        uint16_t CloseSession();
        uint16_t ResetDevice();
        uint16_t PowerDown();
        uint16_t SelfTest(uint16_t type);
        uint16_t GetDeviceInfo(PTPReadParser *parser);
        uint16_t GetDevicePropDesc(const uint16_t pcode, PTPReadParser *parser);
        uint16_t GetDevicePropValue(const uint16_t pcode, PTPReadParser *parser);
        uint16_t GetDevicePropValue(const uint16_t pcode, uint8_t  &val);
        uint16_t GetDevicePropValue(const uint16_t pcode, uint16_t &val);
        uint16_t GetDevicePropValue(const uint16_t pcode, uint32_t &val);
        uint16_t SetDevicePropValue(const uint16_t pcode, uint8_t  val);
        uint16_t SetDevicePropValue(const uint16_t pcode, uint16_t val);
        uint16_t SetDevicePropValue(const uint16_t pcode, uint32_t val);
        uint16_t ResetDevicePropValue(const uint16_t pcode);
        uint16_t GetStorageIDs(PTPReadParser *parser);
        uint16_t GetStorageIDs(uint8_t bufsize, uint8_t *pbuf);
        uint16_t GetStorageInfo(uint32_t storage_id, PTPReadParser *parser);
        uint16_t GetObjectHandles(uint32_t storage_id, uint16_t format, uint16_t assoc, PTPReadParser *parser);
        uint16_t GetObjectInfo(uint32_t handle, PTPReadParser *parser);
        uint16_t FormatStore(uint32_t storage_id, uint32_t fsformat);
        uint16_t GetObject(uint32_t handle, PTPReadParser *parser);
        uint16_t GetThumb(uint32_t handle, PTPReadParser *parser);

        uint16_t GetNumObjects(uint32_t &retval, uint32_t storage_id = 0xffffffff, uint16_t format = 0, uint32_t assoc = 0);
        uint16_t DeleteObject(uint32_t handle, uint16_t format = 0);
        uint16_t SetObjectProtection(uint32_t handle, uint16_t attrib);
        uint16_t MoveObject(uint32_t handle, uint32_t storage_id, uint32_t parent);
        uint16_t CopyObject(uint32_t handle, uint32_t storage_id, uint32_t parent, uint32_t &new_handle);
        uint16_t InitiateOpenCapture(uint32_t storage_id = 0, uint16_t format = 0);
        uint16_t TerminateOpenCapture(uint32_t trans_id);
        uint16_t InitiateCapture(uint32_t storage_id = 0, uint16_t format = 0);

        //uint16_t GetPartialObject(uint32_t handle, PTPReadParser *parser);
        //uint16_t SendObjectInfo(uint32_t handle, PTPDataSupplier *sup);
        //uint16_t SendObject(uint32_t handle, PTPDataSupplier *sup);

private:
        uint8_t GetConfDescr( byte addr, byte conf );
};

#endif // __PTP_H__
