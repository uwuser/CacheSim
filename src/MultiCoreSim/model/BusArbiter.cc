/*
 * File  :      BusArbiter.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 21, 2020
 */

#include "BusArbiter.h"

namespace ns3 {

    // override ns3 type
    TypeId BusArbiter::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::BusArbiter")
               .SetParent<Object > ();
        return tid;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//  
    BusArbiter::BusArbiter(std::list<ns3::Ptr<ns3::BusIfFIFO> > associatedPrivCacheBusIfFIFO,
                           ns3::Ptr<ns3::BusIfFIFO>  assoicateLLCBusIfFIFO,
                           ns3::Ptr<ns3::InterConnectFIFO>  interConnectFIFO) {
        // default
        m_dt                   = (1.0);  // cpu clock cycle period
        m_clkSkew              = 0;
        m_cpuCore              = 4;
        m_arbiCycle            = 0;
        m_reqclks              = 4;
        m_respclks             = 50;
        m_workconserv          = false;
        m_reqCoreCnt           = 0;
        m_respCoreCnt          = 0;
        m_logFileGenEnable     = 0;
        m_cacheBlkSize         = 64;
        m_sharedCacheBusIfFIFO = assoicateLLCBusIfFIFO;
        m_busIfFIFO            = associatedPrivCacheBusIfFIFO;
        m_interConnectFIFO     = interConnectFIFO;
        m_bus_arb              = BusARBType::PISCOT_ARB;
        m_cohProType           = CohProtType::SNOOP_PMSI;
        m_bus_arch             = "split";
        m_bus_arbiter          = "PISCOT";
        m_reqbus_arb           = "TDM";
        m_respbus_arb          = "FCFS";
        m_maxPendingReq        = 1;
        m_PndReq               = false;
        m_PndResp              = false;
        m_cach2Cache           = false;
        m_TimeOut              = 0;
        m_FcFsPndMemResp       = false;
        m_PndPutMChk           = false;
        m_DirectTransfer       = false;
        m_IdleSlot             = false;
        m_PndMemResp           = false;
        m_PndWB                = false;
        for (int i = 0; i < 32; i++) {
          m_ReqWbFlag[i] = true;
        }
        m_stallDetectionEnable = true;
        m_stall_cnt            = 0;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//  
    BusArbiter::~BusArbiter() {
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//  
    void BusArbiter::SetCacheBlkSize (uint32_t cacheBlkSize) {
      m_cacheBlkSize = cacheBlkSize;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//      
    void BusArbiter::SetDt (double dt) {
      m_dt = dt; // dt is cpuClkPeriod
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    int BusArbiter::GetDt () {
      return m_dt;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::SetClkSkew (double clkSkew) {
       m_clkSkew = clkSkew;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::SetIsWorkConserv (bool workConservFlag) {
       m_workconserv = workConservFlag;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::SetNumPrivCore (int nPrivCores) {
      m_cpuCore = nPrivCores; // since we have the same amount of cores as private caches
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::SetNumReqCycles (int ncycle) {
      m_reqclks = ncycle;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::SetNumRespCycles (int ncycle) {
      m_respclks = ncycle;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::SetCache2Cache (bool cach2Cache) {
       m_cach2Cache = cach2Cache;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//     
    void BusArbiter::SetBusArchitecture (string bus_arch) {
      m_bus_arch = bus_arch;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//     
    void BusArbiter::SetBusArbitration (string bus_arb) {
      m_bus_arbiter = bus_arb;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//     
    void BusArbiter::SetReqBusArb (string reqbus_arb) {
      m_reqbus_arb = reqbus_arb;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//    
    void BusArbiter::SetRespBusArb (string respbus_arb) {
      m_respbus_arb = respbus_arb;
    }                 
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//     
    void BusArbiter::SetCohProtType (CohProtType ptype) {
      m_cohProType = ptype;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::SetMaxPendingReq (int maxPendingReq) {
      m_maxPendingReq = maxPendingReq;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//     
    void BusArbiter::SetLogFileGenEnable (bool logFileGenEnable ) {
      m_logFileGenEnable = logFileGenEnable;
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 

   void BusArbiter::SendMemCohrMsg (BusIfFIFO::BusReqMsg msg, bool BroadCast = false) { // send coherence messages on the bus - it seems it only targets the private caches
     for(std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin(); it2 != m_busIfFIFO.end(); it2++) {
       if (msg.reqCoreId == (*it2)->m_fifo_id || BroadCast == true){
         if (!(*it2)->m_rxMsgFIFO.IsFull()) {
           (*it2)->m_rxMsgFIFO.InsertElement(msg);
         }
         else {
           if (m_logFileGenEnable) {
             std::cout << "BusArbiter cannot insert new messages into the buffers" << std::endl;
           }
           exit(0);
         }
       }
     } 
   }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//    
   void BusArbiter::SendData (BusIfFIFO::BusRespMsg msg, AGENT agent) {   // agent is destination that data needs to be sent to and it could be: core, interconnect, shared mem
     // send data to core
     if (agent == AGENT::CORE) {
       for(std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin(); it2 != m_busIfFIFO.end(); it2++) {
         if (msg.reqCoreId == (*it2)->m_fifo_id){
           if (!(*it2)->m_rxRespFIFO.IsFull()) {
             if (m_logFileGenEnable) {
               std::cout << "\nBusArbiter: Cpu/Mem Id " << msg.respCoreId << " Sent Data to Core " <<  msg.reqCoreId << " ============================================== " << m_arbiCycle << "\n\n";
             }
             (*it2)->m_rxRespFIFO.InsertElement(msg);
           }
           else {
             if (m_logFileGenEnable) {
               std::cout << "BusArbiter cannot insert new messages into the buffers" << std::endl;
             }
             exit(0);
           }
         }
       }
     }
     
     if (agent == AGENT::SHAREDMEM) {
       msg.reqCoreId = m_sharedCacheBusIfFIFO->m_fifo_id;
       if (!m_sharedCacheBusIfFIFO->m_rxRespFIFO.IsFull()) {
           m_sharedCacheBusIfFIFO->m_rxRespFIFO.InsertElement(msg);
       }
       else {
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter cannot insert new messages into LLC RxResp buffers" << std::endl;
          }
          exit(0);
       }
     }
     
     if (agent == AGENT::INTERCONNECT){
       if (!m_interConnectFIFO->m_RespMsgFIFO.IsEmpty()) {
         m_interConnectFIFO->m_RespMsgFIFO.PopElement();
       }
       msg.cycle = m_arbiCycle;
       m_interConnectFIFO->m_RespMsgFIFO.InsertElement(msg);
     }
   }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//   where is this interconnect conceptually?  

   bool BusArbiter::FcFsWriteBackCheckInsert (uint16_t core_idx, uint64_t addr, bool CheckOnly, BusIfFIFO::BusRespMsg & txResp) {  //  the WriteBackCheckInsert checks whether the core can send data on the bus.
     bool PendingReq = false;
     std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();
     std::advance(it1, core_idx);

     // get current size of the RxResp queue
     int pendingQueueSize = (*it1)->m_txRespFIFO.GetQueueSize();
     BusIfFIFO::BusRespMsg pendingWbMsg;

     // iterate over buffer to check if the WB message is there
     for (int i = 0; i < pendingQueueSize ;i++) {
       pendingWbMsg = (*it1)->m_txRespFIFO.GetFrontElement();
       // Remove message from the busResp buffer
       (*it1)->m_txRespFIFO.PopElement();
       if (((pendingWbMsg.addr >> (int) log2(m_cacheBlkSize)) == (addr >> (int) log2(m_cacheBlkSize))) && PendingReq == false) {
         if (CheckOnly == true) {
           (*it1)->m_txRespFIFO.InsertElement(pendingWbMsg);
         }
         PendingReq = true;
         txResp = pendingWbMsg;
       }
       else {
         (*it1)->m_txRespFIFO.InsertElement(pendingWbMsg);
       }
     }
     
     // process pending response
     if (PendingReq == true && CheckOnly == false) {
       if (m_logFileGenEnable) {
         std::cout << "\nBusArbiter: Cpu " << (*it1)->m_fifo_id << " granted TDM response slot =============================================================== " << m_arbiCycle << "\n\n";
       }
       SendData (txResp, AGENT::CORE);

       if (txResp.reqCoreId == m_sharedCacheBusIfFIFO->m_fifo_id || txResp.dualTrans == true) {
         if (m_logFileGenEnable) {
           std::cout << "\nBusArbiter: Cpu " << (*it1)->m_fifo_id << " Sent Data to Shared Mem =============================================================== " << m_arbiCycle << "\n\n";
         }
         SendData (txResp, AGENT::SHAREDMEM); 
       }
       SendData (txResp, AGENT::INTERCONNECT); 
       return true; 
     }
     return PendingReq;
   }
// what is the goal in this function? Is it checking if an specific RespMsg inserted in the shared memory buffers? if yes, why it is checking
// the txRespFIFO of the sharedmemctrl? is it from my perspective that the WB is inserted in the TX?
   bool BusArbiter::FcFsMemCheckInsert (uint16_t coreId, uint64_t addr, bool CheckOnly = true, bool SkipAddrCheck = false) { // FcFsMemCheckInsert checks If the shared memory can send data on the bus
     bool PendingReq = false;
     int pendingQueueSize = m_sharedCacheBusIfFIFO->m_txRespFIFO.GetQueueSize();
     BusIfFIFO::BusRespMsg pendingWbMsg, txResp;

     for (int i = 0; i < pendingQueueSize ;i++) {
       pendingWbMsg = m_sharedCacheBusIfFIFO->m_txRespFIFO.GetFrontElement();
       m_sharedCacheBusIfFIFO->m_txRespFIFO.PopElement();
       if (pendingWbMsg.reqCoreId == coreId && (pendingWbMsg.addr == addr || SkipAddrCheck == true)) {
         if (CheckOnly == true) {
           m_sharedCacheBusIfFIFO->m_txRespFIFO.InsertElement(pendingWbMsg);
         }
         PendingReq = true;
         txResp = pendingWbMsg;
       }
       else {
         m_sharedCacheBusIfFIFO->m_txRespFIFO.InsertElement(pendingWbMsg);
       }
     }
     if (PendingReq == true && CheckOnly == false) {
       if (m_logFileGenEnable) {
         std::cout << "BusArbiter: SharedMem granted TDM response slot ============================================================ " << m_arbiCycle << "\n\n";
       }
       
       SendData (txResp, AGENT::CORE);
       
       SendData (txResp, AGENT::INTERCONNECT); 

       return true; 
     }
     return PendingReq;
   }
// same points as previous one regarding why this buffer is checked


   bool BusArbiter::CheckPendingFCFSReq (BusIfFIFO::BusReqMsg & txMsg, bool ChkOnly = true) {   /* checks if there is a  pending request in FCFS among all requestors not for specific core. */ 
     double arrivalTime  = 0;
     uint16_t core_idx   = 0;
     uint16_t TargetCore = 0;
     bool PendingTxReq   = false;

     for(std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin(); it1 != m_busIfFIFO.end(); it1++) {
       if ((*it1)->m_txMsgFIFO.IsEmpty()==false) {
         txMsg = (*it1)->m_txMsgFIFO.GetFrontElement();
         if ((PendingTxReq == false) || arrivalTime > txMsg.timestamp){
           arrivalTime = txMsg.timestamp;
           TargetCore  = core_idx;
         }
         PendingTxReq = true;
       }
       core_idx++;
     }

     if (PendingTxReq) {
       std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin();
       std::advance(it2, TargetCore);
       txMsg = (*it2)->m_txMsgFIFO.GetFrontElement();
       // Remove message from the busResp buffer
       if (ChkOnly == false) {
         (*it2)->m_txMsgFIFO.PopElement();
       }
     }
     return PendingTxReq;
   }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//    
   bool BusArbiter::CheckPendingFCFSResp (BusIfFIFO::BusRespMsg & txMsg, bool ChkOnly = true) {
     double arrivalTime  = 0;
     uint16_t core_idx   = 0;
     uint16_t TargetCore = 0;
     bool PendingTxReq   = false;

     for(std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin(); it1 != m_busIfFIFO.end(); it1++) {
       if ((*it1)->m_txRespFIFO.IsEmpty()==false) {
         txMsg = (*it1)->m_txRespFIFO.GetFrontElement();
         if ((PendingTxReq == false) || arrivalTime > txMsg.timestamp){
           arrivalTime = txMsg.timestamp;
           TargetCore  = core_idx;
         }
         PendingTxReq = true;
       }
       core_idx++;
     }

     BusIfFIFO::BusRespMsg txMsgMem;
     if (m_sharedCacheBusIfFIFO->m_txRespFIFO.IsEmpty()==false) {
       txMsgMem = m_sharedCacheBusIfFIFO->m_txRespFIFO.GetFrontElement();
       if ((PendingTxReq == false) || arrivalTime > txMsgMem.timestamp){
         arrivalTime = txMsgMem.timestamp;
         TargetCore  = m_sharedCacheBusIfFIFO->m_fifo_id;
       }
       PendingTxReq = true;
     }

     if (PendingTxReq) {
       if (TargetCore == m_sharedCacheBusIfFIFO->m_fifo_id) {
         txMsg = txMsgMem;
         if (ChkOnly == false) {
           m_sharedCacheBusIfFIFO->m_txRespFIFO.PopElement();
         }
       }
       else {
         std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin();
         std::advance(it2, TargetCore);
         txMsg = (*it2)->m_txRespFIFO.GetFrontElement();
         // Remove message from the busResp buffer
         if (ChkOnly == false) {
           (*it2)->m_txRespFIFO.PopElement();
         }
       }
       
       if (ChkOnly == false) {
         if (m_logFileGenEnable) {
           if (TargetCore == m_sharedCacheBusIfFIFO->m_fifo_id) {
             std::cout << "\nBusArbiter: SharedMem " << m_sharedCacheBusIfFIFO->m_fifo_id << " granted TDM response slot =============================================================== " << m_arbiCycle << "\n\n";
           }
           else {
             std::cout << "\nBusArbiter: Cpu " << TargetCore << " granted TDM response slot =============================================================== " << m_arbiCycle << "\n\n";
           }
         }
         
         SendData (txMsg, AGENT::CORE);

         if (txMsg.reqCoreId == m_sharedCacheBusIfFIFO->m_fifo_id || txMsg.dualTrans == true) {
           if (m_logFileGenEnable) {
             std::cout << "\nBusArbiter: Cpu " << TargetCore << " Sent Data to Shared Mem =============================================================== " << m_arbiCycle << "\n\n";
           }
           SendData (txMsg, AGENT::SHAREDMEM); 
         }
       
         SendData (txMsg, AGENT::INTERCONNECT); 
       }
     }
     return PendingTxReq;
   }
// it seems for the response, we should check both the BusIFFIFO and also sharedCacheBusIfFIFO. --> why?  Because pending data can come either from cores (in case of cache to cache) or from shared cache itself.
// what is the reason of line 360 to 371? we have the same scenario in line 224 to 232 : To my understanding, each of them sends the response to a different destination based on the designated message’s desintation.
   bool BusArbiter::CheckPendingReq (uint16_t core_idx, BusIfFIFO::BusReqMsg & txMsg, bool CheckOnly = false) {
     std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();
     std::advance(it1, core_idx);
     bool PendingTxReq = false;

     if (!(*it1)->m_txMsgFIFO.IsEmpty()) {
       PendingTxReq = true;
       txMsg = (*it1)->m_txMsgFIFO.GetFrontElement();
       if (PendingTxReq && CheckOnly == false) {
         // Remove message from the busResp buffer
         (*it1)->m_txMsgFIFO.PopElement();
       }
     }
     return PendingTxReq;
   }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
   bool BusArbiter::CheckPendingReqNonOldest (uint16_t core_idx, BusIfFIFO::BusReqMsg & txMsg, bool CheckOnly = false){
     /************************** Under Developement *******************************/
     BusIfFIFO::BusReqMsg txResp;
     std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();     
     std::advance(it1, core_idx);
     bool PendingTxReq = false;
     bool candidate_chosen = true;
     int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();
     if (!(*it1)->m_txMsgFIFO.IsEmpty())
     {       
       for (unsigned int iterator = 1; (iterator < (*it1)->m_txMsgFIFO.GetQueueSize() && candidate_chosen == true) ; iterator++)
       {
         PendingTxReq = true;
         txMsg = (*it1)->m_txMsgFIFO.GetElement(iterator);

         for (int j = 0; (j < pendingQueueSize && PendingTxReq == true) ;j++) 
          {
            txResp = m_GlobalReqFIFO.GetElement(j);
            m_GlobalReqFIFO.PopElement();
            if (it1 == txResp.reqCoreId) { /*************** NEEDS MODIFICATION SINCE NOW WE HAVE MORE BANKS! HENCE, WE SHOULD CHECK IF THAT RESPONSE IS FOR THAT SPECIFIC BANK OR NOT? CORRECT? ***************/
              PendingTxReq = false;
            }
            m_GlobalReqFIFO.InsertElement(txResp);
          } 
          if(PendingTxReq) { candidate_chosen = false;} 
       }
     }
     return PendingTxReq;
   }
   
   bool BusArbiter::CheckPendingWB (uint16_t core_idx, BusIfFIFO::BusRespMsg & wbMsg, bool CheckOnly = false) {
     std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin();
     std::advance(it1, core_idx);
     bool PendingWbFlag = false;

     if (!(*it1)->m_txRespFIFO.IsEmpty()) {
       PendingWbFlag = true;
       wbMsg = (*it1)->m_txRespFIFO.GetFrontElement();
       if (PendingWbFlag && CheckOnly == false) {
         // Remove message from the busResp buffer
         (*it1)->m_txRespFIFO.PopElement();
       }
     }
     return PendingWbFlag;
   }
// the same question as  in  FcFsWriteBackCheckInsert. why m_txRespFIFO? so it is
// in txResp of the bus.. hummm check it later 

   bool BusArbiter::CheckPendingPutM (BusIfFIFO::BusReqMsg reqMsg, BusIfFIFO::BusReqMsg & putmReq) {
     BusIfFIFO::BusReqMsg txreq;
     int pendingQueueSize;
     bool PendingPutmReq = false;

     for(std::list<Ptr<BusIfFIFO>>::iterator it1 = m_busIfFIFO.begin(); it1 != m_busIfFIFO.end() && (PendingPutmReq == false); it1++) {
       // check if core has pending requests
       pendingQueueSize = (*it1)->m_txMsgFIFO.GetQueueSize();
       for (int i = 0; i < pendingQueueSize ;i++) {
         txreq = (*it1)->m_txMsgFIFO.GetFrontElement();
         // Remove message from the busReq buffer
         (*it1)->m_txMsgFIFO.PopElement();
         if (txreq.cohrMsgId == SNOOPPrivCohTrans::PutMTrans && (txreq.addr == reqMsg.addr) && (txreq.reqCoreId == reqMsg.reqCoreId)) {
           putmReq = txreq;
           PendingPutmReq = true;
         }
         else if (txreq.cohrMsgId == SNOOPPrivCohTrans::PutMTrans && 
            ((txreq.addr >> (int) log2(m_cacheBlkSize)) == (reqMsg.addr >> (int) log2(m_cacheBlkSize))) &&
            (txreq.msgId == 0)) {
           putmReq = txreq;
           putmReq.addr      = reqMsg.addr;
           putmReq.msgId     = reqMsg.msgId;
           putmReq.reqCoreId = reqMsg.reqCoreId;
           PendingPutmReq    = true;
         }
         else {
           (*it1)->m_txMsgFIFO.InsertElement(txreq);
         }
       }
     }
     return PendingPutmReq;  
   }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
   bool BusArbiter::InsertOnReqBus (BusIfFIFO::BusReqMsg txMsg) { // it takes a coherence message that is picked by the requestBus Arbiter and broadcasts it.
     if (m_logFileGenEnable) {
       if ( txMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans){
         std::cout << "\nBusArbiter: Cpu " << txMsg.wbCoreId << " granted TDM request slot =============================================================== " << m_arbiCycle <<
         std::endl;
       }
       else {
         std::cout << "\nBusArbiter: Cpu " << txMsg.reqCoreId << " granted TDM request slot =============================================================== " << m_arbiCycle <<
         std::endl;
       }
     }
     // broadcast requests to all cores (snooping based)
     for(std::list<Ptr<BusIfFIFO>>::iterator it2 = m_busIfFIFO.begin(); it2 != m_busIfFIFO.end(); it2++) {
       if (!(*it2)->m_rxMsgFIFO.IsFull()) {
         (*it2)->m_rxMsgFIFO.InsertElement(txMsg);
       }
       else {
         if (m_logFileGenEnable) {
           std::cout << "BusArbiter: cannot insert new messages into PrivCache Ctrl buffers" << std::endl;
         }
         exit(0);
       }
     }
     // send request to Shared Mem controller as well
     if (!m_sharedCacheBusIfFIFO->m_rxMsgFIFO.IsFull()) {
       m_sharedCacheBusIfFIFO->m_rxMsgFIFO.InsertElement(txMsg);
     }
     else {
       if (m_logFileGenEnable) {
         std::cout << "BusArbiter: cannot insert new messages into SharedMem RxMsg buffers" << std::endl;
       }
       exit(0);
     }
     // send message to Interconnect FIFO
     if (!m_interConnectFIFO->m_ReqMsgFIFO.IsEmpty()) {
       m_interConnectFIFO->m_ReqMsgFIFO.PopElement();
     }
     txMsg.cycle = m_arbiCycle;
     m_interConnectFIFO->m_ReqMsgFIFO.InsertElement(txMsg);
     return true;
   }
// seems fine, broadcasting to all cores and also the shared mem controller rxMsgFIFO buffers. Just again, not sure about the interconnet and its role?
    void BusArbiter::PMSI_TDM_ReqBus() {  
      BusIfFIFO::BusReqMsg tempPutmMsg;     
      if (m_PndReq) {
        InsertOnReqBus (m_ReqBusMsg);
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        m_PndPutMChk = true;
        m_PndReq = false;
        return;
      }

      if (m_PndPutMChk) {
        if (CheckPendingPutM(m_ReqBusMsg, tempPutmMsg)){
          InsertOnReqBus (tempPutmMsg);
          m_GlobalReqFIFO.InsertElement(tempPutmMsg);
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Insert Put(M) on the Bus from Core " << tempPutmMsg.wbCoreId <<"============================================================\n\n";
          }
        }

        if (m_ReqBusMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) {
          int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();
          bool InsertionDone = false;
          BusIfFIFO::BusReqMsg txResp;
          for (int i = 0; i < pendingQueueSize ;i++) {
            txResp = m_GlobalReqFIFO.GetFrontElement();
            // Remove message from the busResp buffer
            m_GlobalReqFIFO.PopElement();

            if (InsertionDone == false &&(txResp.cohrMsgId == SNOOPPrivCohTrans::GetSTrans || SNOOPPrivCohTrans::GetMTrans) && 
               ((txResp.addr >> (int) log2(m_cacheBlkSize)) ==  (m_ReqBusMsg.addr >> (int) log2(m_cacheBlkSize))) ) {
              m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
              m_GlobalReqFIFO.InsertElement(txResp);
              InsertionDone = true;
            }
            else {
              m_GlobalReqFIFO.InsertElement(txResp);
            }
          }

          if (InsertionDone == false) {
            m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
          }
        }
        else if (m_ReqBusMsg.cohrMsgId != SNOOPPrivCohTrans::UpgTrans) {
          m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
        }

        // advance TDM slot pointer
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
        m_PndPutMChk = false;
      }


      for (int i = 0; i < m_cpuCore && m_PndReq == false; i++) {
        m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg);
        if (!m_PndReq)
          m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
      }

      if (m_PndReq) {
        // wait one TDM Request slot
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else {
        // wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
    }// void BusArbiter::PMSI_TDM_ReqBus()
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// NEVER USED
    // Unified TDM bus 
    void BusArbiter::Unified_TDM_PMSI_Bus2 () {
    
      BusIfFIFO::BusReqMsg tempPutmMsg;
      bool PutMFromOtherCore;
      uint16_t IdleSlotWidth;

      // do write back to memory or core
      // global queue actions (TDM response slot)
      if (m_PndMemResp || m_PndWB) {
        if (m_PndMemResp){
          FcFsMemCheckInsert (m_reqCoreCnt, 0, false, true);
        }
        else {
           SendData (m_PendWBMsg, AGENT::SHAREDMEM); 
        }
                                                        
        m_PndMemResp = false;
        m_PndWB      = false;
                                                        
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);
        Simulator::Schedule(NanoSeconds(m_dt*2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        return;
      }
      else if (m_PndReq) {
        InsertOnReqBus (m_ReqBusMsg);
        
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        m_PndPutMChk = true;
        m_PndReq = false;
        return;
      }
      else if (m_PndPutMChk) {
        PutMFromOtherCore = CheckPendingPutM(m_ReqBusMsg, tempPutmMsg);
        if (PutMFromOtherCore){
          InsertOnReqBus (tempPutmMsg);
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Insert Put(M) on the Bus from Core " << tempPutmMsg.wbCoreId <<"============================================================\n\n";
          }
          m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);    
        }
        else {
          m_DirectTransfer = true;
        }
        
        m_PndPutMChk = false;  
              
        Simulator::Schedule(NanoSeconds(m_dt*(m_respclks-m_reqclks)), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        return;
      }
      else if (m_DirectTransfer) {
        if (m_ReqBusMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) {
          FcFsWriteBackCheckInsert (m_reqCoreCnt, m_ReqBusMsg.addr, false, m_PendResp) ;
          if (m_logFileGenEnable) {
          std::cout << "BusArbiter: Direct Core replacement from Core: " << m_reqCoreCnt <<" ============================================== " << m_arbiCycle << "\n\n";
          }  
        }
        else {
          if (!FcFsMemCheckInsert(m_reqCoreCnt, m_ReqBusMsg.addr, false)){
            if (m_logFileGenEnable) {
              std::cout << "BusArbiter: Not ready direct Memory Transfer to Core: " << m_reqCoreCnt <<" ============================================== " << m_arbiCycle << "\n\n";
            }  
          }
        }
        m_DirectTransfer = false; 
        
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);    
      }
      else if (m_IdleSlot) {
        m_IdleSlot = false;
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);          
      }
      
      m_IdleSlot = true;
             
      m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, true);
      
      m_PndWB  = CheckPendingWB (m_reqCoreCnt, m_PendWBMsg, true);
      
      m_PndMemResp = FcFsMemCheckInsert (m_reqCoreCnt, 0, true, true);
      
      if ((m_PndReq || m_PndMemResp) && m_PndWB) {
        if (m_logFileGenEnable) {
          std::cout << "BusArbiter: Multiple request from core " << m_reqCoreCnt <<" ===================================================" << m_arbiCycle << "\n\n";
        }
        if (m_ReqWbFlag[m_reqCoreCnt]) {
           m_PndWB = false;
        }
        else {
          m_PndReq = false;
          m_PndMemResp = false;
        }      
      }

      if (m_PndReq && m_PndMemResp) {
        m_PndReq = false;
      }
      
      if (m_PndReq || m_PndMemResp || m_PndWB) {
        m_IdleSlot = false;
        m_ReqWbFlag[m_reqCoreCnt] = (m_PndWB == true) ? true : false;
      }
      else {
        m_ReqWbFlag[m_reqCoreCnt] = true;
      }
             
      if (m_PndReq == true) {
         m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, false);
        if (m_logFileGenEnable) {                                           
          std::cout << "BusArbiter: Request Slot from core " << m_reqCoreCnt <<" ===================================================" << m_arbiCycle << "\n\n";
        }
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else {
        if (m_IdleSlot == true) {
          IdleSlotWidth = (m_workconserv == true)  ? 1 : m_respclks;
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Idle Slot for core " << m_reqCoreCnt <<" ===================================================" << m_arbiCycle << "\n\n";
          }
          Simulator::Schedule(NanoSeconds(m_dt*IdleSlotWidth), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        }
        else {
          if (m_PndWB) {
            m_PndWB  = CheckPendingWB (m_reqCoreCnt, m_PendWBMsg, false);
          }
          Simulator::Schedule(NanoSeconds(m_dt*(m_respclks-2)), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        }
      }
                
    }
// Not sure about this Unified_TDM_PMSI_Bus2() at this point ------------------------------------    
    // Unified TDM bus (PMSI only)
    void BusArbiter::Unified_TDM_PMSI_Bus () {
      // global queue msg
      BusIfFIFO::BusReqMsg glb_queue, tempPutmMsg;
      bool PutMFromOtherCore;
      uint16_t IdleSlotWidth;

      if (m_PndResp || m_FcFsPndMemResp) {
        glb_queue = m_GlobalReqFIFO.GetFrontElement(); 

        m_GlobalReqFIFO.PopElement();
        
        m_FcFsPndMemResp = (m_FcFsPndMemResp == true) ? FcFsMemCheckInsert       (m_reqCoreCnt, glb_queue.addr, false) :
                                                        FcFsWriteBackCheckInsert (m_reqCoreCnt, glb_queue.addr, false, m_PendResp) ;
                                                        
        m_PndResp        = false;
        m_FcFsPndMemResp = false;
                                                        
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);
        Simulator::Schedule(NanoSeconds(m_dt*2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        return;
      }
      else if (m_PndReq) {
        InsertOnReqBus (m_ReqBusMsg);
        
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        m_PndPutMChk = true;
        m_PndReq = false;
        return;
      }
      else if (m_PndPutMChk) {
        PutMFromOtherCore = CheckPendingPutM(m_ReqBusMsg, tempPutmMsg);
        if (PutMFromOtherCore){
          InsertOnReqBus (tempPutmMsg);
          m_GlobalReqFIFO.InsertElement(tempPutmMsg);
          m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Insert Put(M) on the Bus from Core " << tempPutmMsg.wbCoreId <<"============================================================\n\n";
          }
          m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);    
        }
        else {
          m_DirectTransfer = true;
        }
        
        m_PndPutMChk = false;  
              
        Simulator::Schedule(NanoSeconds(m_dt*(m_respclks-m_reqclks)), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        return;
      }
      else if (m_DirectTransfer) {
        if (m_ReqBusMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) {
          FcFsWriteBackCheckInsert (m_reqCoreCnt, m_ReqBusMsg.addr, false, m_PendResp) ;
          if (m_logFileGenEnable) {
          std::cout << "BusArbiter: Direct Core replacement: " << m_reqCoreCnt <<" ============================================== " << m_arbiCycle << "\n\n";
          }  
        }
        else {
          if (!FcFsMemCheckInsert(m_reqCoreCnt, m_ReqBusMsg.addr, false)){
            m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
            if (m_logFileGenEnable) {
              std::cout << "BusArbiter: Direct Memory Transfer: " << m_reqCoreCnt <<" ============================================== " << m_arbiCycle << "\n\n";
            }  
          }
        }
        m_DirectTransfer = false; 
        
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);    
      }
      else if (m_IdleSlot) {
        m_IdleSlot = false;
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1);          
      }
      
      m_IdleSlot = true;
     
      if (!m_GlobalReqFIFO.IsEmpty()) {
        glb_queue = m_GlobalReqFIFO.GetFrontElement();
        
        if (glb_queue.wbCoreId == m_reqCoreCnt && glb_queue.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) {
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Response Slot, Wb slot from core " << m_reqCoreCnt <<"===================================================" << m_arbiCycle << "\n\n";
           }
          m_PndResp  = true;
        }
        
        if (glb_queue.reqCoreId == m_reqCoreCnt && glb_queue.cohrMsgId != SNOOPPrivCohTrans::PutMTrans) {
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Response Slot, Memory Transfer slot to core " << m_reqCoreCnt <<"==================================================" << m_arbiCycle << "\n\n";
          }
          m_FcFsPndMemResp = true; 
        }
      }
      
      m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, true);
      
      if ((m_FcFsPndMemResp || m_PndResp) && m_PndReq) {
        if (m_ReqWbFlag[m_reqCoreCnt]) {
           m_PndResp = false;
           m_FcFsPndMemResp = false;
        }
        else {
          m_PndReq = false;
        }
      }  
      
      if (m_FcFsPndMemResp || m_PndResp || m_PndReq) {
        m_IdleSlot = false;
        m_ReqWbFlag[m_reqCoreCnt] = (m_PndResp == true) ? true : false;
      }
              
      if (m_PndReq == true) {
         m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg);
        if (m_logFileGenEnable) {                                           
          std::cout << "BusArbiter: Request Slot from core " << m_reqCoreCnt <<" ===================================================" << m_arbiCycle << "\n\n";
        }
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else {
        if (m_IdleSlot == true) {
          IdleSlotWidth = (m_workconserv == true)  ? 1 : m_respclks;
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Idle Slot for core " << m_reqCoreCnt <<" ===================================================" << m_arbiCycle << "\n\n";
          }
          Simulator::Schedule(NanoSeconds(m_dt*IdleSlotWidth), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        }
        else {
          Simulator::Schedule(NanoSeconds(m_dt*(m_respclks-2)), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        }
      }
         
    }// void BusArbiter::Unified_TDM_PMSI_Bus
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// NEVER USED
    void BusArbiter::PMSI_OOO_TDM_ReqBus() {  
      BusIfFIFO::BusReqMsg tempPutmMsg;     
      if (m_PndReq) {
        InsertOnReqBus (m_ReqBusMsg);
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        m_PndPutMChk = true;
        m_PndReq = false;
        return;
      }

      if (m_PndPutMChk) {
        // check if there is a pending PutM issued from other cores
        // This PutM has to broatcasted independed of the Bus 
        // Arbiteration Policy.
        if (CheckPendingPutM(m_ReqBusMsg, tempPutmMsg)){
          InsertOnReqBus (tempPutmMsg);
          m_GlobalReqFIFO.InsertElement(tempPutmMsg);
          if (m_logFileGenEnable) {
            std::cout << "BusArbiter: Insert Put(M) on the Bus from Core " << tempPutmMsg.wbCoreId <<"============================================================\n\n";
          }
        }

        // sort Resquest Messages
        if (m_ReqBusMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) {
          int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();
          bool InsertionDone = false;
          BusIfFIFO::BusReqMsg txResp;
          for (int i = 0; i < pendingQueueSize ;i++) {
            txResp = m_GlobalReqFIFO.GetFrontElement();
            // Remove message from the busResp buffer
            m_GlobalReqFIFO.PopElement();

            if (InsertionDone == false &&(txResp.cohrMsgId == SNOOPPrivCohTrans::GetSTrans || SNOOPPrivCohTrans::GetMTrans) && 
               ((txResp.addr >> (int) log2(m_cacheBlkSize)) ==  (m_ReqBusMsg.addr >> (int) log2(m_cacheBlkSize))) ) {
              m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
              m_GlobalReqFIFO.InsertElement(txResp);
              InsertionDone = true;
            }
            else {
              m_GlobalReqFIFO.InsertElement(txResp);
            }
          }

          if (InsertionDone == false) {
            m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
          }
        }
        else if (m_ReqBusMsg.cohrMsgId != SNOOPPrivCohTrans::UpgTrans) {
          m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
        }

        // advance TDM slot pointer
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
        m_PndPutMChk = false;
      }

      // check pending request
      bool temp_PndReq = false;
      int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();
      BusIfFIFO::BusReqMsg txResp;

      // OOO TDM implementation
      for (int i = 0; i < m_cpuCore && m_PndReq == false; i++) {
        // check if the current candidate has a request in its local buffer
        temp_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, true);
        m_PndReq    = temp_PndReq;

        // check if that core has a pending request in the service queue
        for (int j = 0; (j < pendingQueueSize && temp_PndReq == true && m_ReqBusMsg.cohrMsgId != SNOOPPrivCohTrans::PutMTrans) ;j++) {
          txResp = m_GlobalReqFIFO.GetFrontElement();
          m_GlobalReqFIFO.PopElement();
          if (m_reqCoreCnt == txResp.reqCoreId) {
            m_PndReq = false;
          }
          m_GlobalReqFIFO.InsertElement(txResp);
        }
        // advance current candidate if it doesn't have request
        if (!m_PndReq) {
          m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
        }
        else { // fetch request message
          m_PndReq     = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, false);
        }
      }

      if (m_PndReq) {
        // wait one TDM Request slot
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks/2), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else {
        // wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
    }// void BusArbiter::PMSI_OOO_TDM_ReqBus()
// Not sure about this PMSI_OOO_TDM_ReqBus() at this point  ------------------------------------         
    void BusArbiter::PISCOT_MSI_TDM_ReqBus() {  
      // send message on request bus
      if (m_PndReq) {
        InsertOnReqBus (m_ReqBusMsg);
        m_PndPutMChk = true;
        m_PndReq = false;
      }
      // insert message into service queue
      if (m_PndPutMChk) {
        // check if PutM need to be inserted
        if (m_ReqBusMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) {
          int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();
          bool InsertionDone = false;
          BusIfFIFO::BusReqMsg txResp;
          for (int i = 0; i < pendingQueueSize ;i++) {
            txResp = m_GlobalReqFIFO.GetFrontElement();
            // Remove message from the busResp buffer
            m_GlobalReqFIFO.PopElement();

            if (InsertionDone == false &&(txResp.cohrMsgId == SNOOPPrivCohTrans::GetSTrans || SNOOPPrivCohTrans::GetMTrans) && 
               ((txResp.addr >> (int) log2(m_cacheBlkSize)) ==  (m_ReqBusMsg.addr >> (int) log2(m_cacheBlkSize))) ) {
              InsertionDone = true;
            }
            // dequeuing
            m_GlobalReqFIFO.InsertElement(txResp);
          }
          if (InsertionDone == false) {
            m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
          }
        }
        else if (m_ReqBusMsg.cohrMsgId != SNOOPPrivCohTrans::UpgTrans) {
          m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
        }
        // advance TDM slot pointer
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
        m_PndPutMChk = false;
      }

      // TDM request bus implementation
      for (int i = 0; i < m_cpuCore && m_PndReq == false; i++) {
        m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg);
        if (!m_PndReq)
          m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
      }

      if (m_PndReq) {
        // wait one TDM Request slot
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else {
        // wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
    }// void BusArbiter::PISCOT_MSI_TDM_ReqBus()
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// NEVER USED    
    void BusArbiter::PISCOT_OOO_MSI_TDM_ReqBus() {  
      BusIfFIFO::BusReqMsg txResp;
      // insert message on the request bus (broadcast)
      // in the slot, if there was a pending request chosen before, now its time to broadcast. and finally advance the TDM
      if (m_PndReq) 
      {
        m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, false);
        m_PndReq = false;
        // insert message on request bus
        InsertOnReqBus (m_ReqBusMsg);
        // insert message into service queue
        if (m_ReqBusMsg.NoGetMResp == false)                                       /// what is NoGetMResp?
        { 
          m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
        }
        // advance TDM slot pointer
        m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
      }


      // either if there was no pending requst or it was broadcasted, we should check for the next candidate
      // first check if the TDM candidate has anything in its queue
      // if yes, take it from the global service pending queue and store it
      // then advance TDM slot - if didn't find anything, advance until you find something --> it is work conserving then????  
      // check pending request
      bool temp_PndReq = false;
      int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();

      // OOO TDM implementation
      for (int i = 0; i < m_cpuCore && m_PndReq == false; i++) 
      {
        // check if the current candidate has a request in its local buffer
        temp_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, true);
        m_PndReq    = temp_PndReq;

        // check if that core has a pending request in the service queue
        for (int j = 0; (j < pendingQueueSize && temp_PndReq == true) ;j++) 
        {
          txResp = m_GlobalReqFIFO.GetFrontElement();
          m_GlobalReqFIFO.PopElement();
          if (m_reqCoreCnt == txResp.reqCoreId) {
            m_PndReq = false;
          }
          m_GlobalReqFIFO.InsertElement(txResp);
        }
        // the MSG will not be removed from the txBuffer until it is ready to broadcast. 
        // Therefore, we only remove it from the GlobalReqFifo and then upon broadcasting we will remove it by checkpendingReq(false checking)
        // advance current candidate if it doesn't have request
        if (!m_PndReq) {
          m_reqCoreCnt = (m_reqCoreCnt == m_cpuCore - 1) ? 0 : (m_reqCoreCnt + 1); 
        }
      }
      // wait one TDM Request slot, if there is any request
      if (m_PndReq) {
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else {// wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
    } // void BusArbiter::PISCOT_OOO_MSI_TDM_ReqBus() {  
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::MSI_FcFsReqBus() {  
      // insert message on the request bus
      if (m_PndReq) {
        m_PndReq = CheckPendingFCFSReq (m_ReqBusMsg, false);
        InsertOnReqBus(m_ReqBusMsg);
        m_PndPutMChk = true;
        m_PndReq     = false;
      }
      // insert request message into service queue
      if (m_PndPutMChk) {
        int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();
        // Insert PutM message if it is due to eviction only
        if (m_ReqBusMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans && m_ReqBusMsg.msgId == 0) {
          bool PutInsertionDone = false;
          BusIfFIFO::BusReqMsg txReq;
          // iterate over service queue
          for (int i = 0; i < pendingQueueSize ;i++) {
            txReq = m_GlobalReqFIFO.GetFrontElement();
            m_GlobalReqFIFO.PopElement();
            if (PutInsertionDone == false &&(txReq.cohrMsgId == SNOOPPrivCohTrans::GetSTrans || SNOOPPrivCohTrans::GetMTrans) && 
               ((txReq.addr >> (int) log2(m_cacheBlkSize)) ==  (m_ReqBusMsg.addr >> (int) log2(m_cacheBlkSize))) ) {
              if (m_logFileGenEnable) {
                std::cout << "Skip PutM insertion into global buffer, This should not happen " << txReq.reqCoreId << std::endl;
              }
              PutInsertionDone = true;
            }
            // dequeuing 
            m_GlobalReqFIFO.InsertElement(txReq);
          }
          // insert PutM if it was due to eviction
          if (PutInsertionDone == false) {
            m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
          }
        }
        else if (m_ReqBusMsg.cohrMsgId != SNOOPPrivCohTrans::UpgTrans) { // insert other requests as normal
          if (m_ReqBusMsg.NoGetMResp == false) { // Upgrade from "E" to "M" in MESI or MOESI 
            m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
          }
        }
        m_PndPutMChk = false;
      }
      m_PndReq = CheckPendingFCFSReq (m_ReqBusMsg);
      // wait one Req-TDM slot
      if (m_PndReq) {
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else { // wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
    } //void BusArbiter::MSI_FcFsReqBus()
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// NEVER USED    
    void BusArbiter::MSI_FcFsReqBus2() {  
      // insert message on the request bus
      // it takes the oldest no matter which core
      // it does not have any TDM slot so not advancing
      if (m_PndReq) {
        m_PndReq = CheckPendingFCFSReq (m_ReqBusMsg, false);
        InsertOnReqBus(m_ReqBusMsg);
        m_PndReq     = false;
      }
      
      m_PndReq = CheckPendingFCFSReq (m_ReqBusMsg);
      // wait one Req-TDM slot
      if (m_PndReq) {
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else { // wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
    } //void BusArbiter::MSI_FcFsReqBus2()
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//     
    void BusArbiter::MSI_FcFsRespBus() {  
      if (m_PndResp) {
        m_PndResp = CheckPendingFCFSResp (m_PendResp, false);
        m_PndResp     = false;
      }
      
      m_PndResp = CheckPendingFCFSResp (m_PendResp);
      if (m_PndResp) {
        Simulator::Schedule(NanoSeconds(m_dt*m_respclks), &BusArbiter::RespStep, Ptr<BusArbiter > (this));
      }
      else { // wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::RespStep, Ptr<BusArbiter > (this));
      }
    } //void BusArbiter::MSI_FcFsRespBus()
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::PMSI_FcFsRespBus() {
      if (m_PndResp || m_FcFsPndMemResp) {
        m_FcFsPndMemResp = (m_FcFsPndMemResp == true) ? FcFsMemCheckInsert       (m_respCoreCnt, m_ServQueueMsg.addr, false) :
                                                        FcFsWriteBackCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr, false, m_PendResp) ;
        m_GlobalReqFIFO.PopElement();
        if (m_cach2Cache && m_PndResp && m_ServQueueMsg.msgId != 0) {
          m_GlobalReqFIFO.PopElement();
        }
      }
      m_PndResp        = false;
      m_FcFsPndMemResp = false;
      if (!m_GlobalReqFIFO.IsEmpty()) {
        m_ServQueueMsg = m_GlobalReqFIFO.GetFrontElement();
        if (m_ServQueueMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) {
          m_respCoreCnt = m_ServQueueMsg.wbCoreId;
          m_PndResp = FcFsWriteBackCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr, true, m_PendResp);
        }
        else { // this is a memory transfer slot
          m_respCoreCnt = m_ServQueueMsg.reqCoreId;
          m_FcFsPndMemResp = FcFsMemCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr);
        }
      }
      if (m_PndResp == true || m_FcFsPndMemResp == true) {
        Simulator::Schedule(NanoSeconds(m_dt*m_respclks), &BusArbiter::RespStep, Ptr<BusArbiter > (this));
      }
      else { // wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::RespStep, Ptr<BusArbiter > (this));
      }
    } //void BusArbiter::PMSI_FcFsRespBus() 
//^^^^^^^^^^^^^^^^^^^^^^^NOT REWIEWD FOR NOW^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::PISCOT_MSI_FcFsResBus() {
      if (m_PndResp || m_FcFsPndMemResp) 
      {
        m_FcFsPndMemResp = (m_FcFsPndMemResp == true) ? FcFsMemCheckInsert       (m_respCoreCnt, m_ServQueueMsg.addr, false) :
                                                        FcFsWriteBackCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr, false, m_PendResp);
        if ((m_cach2Cache == false &&  ((m_PndResp == false) || (m_ServQueueMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans))) 
            || (m_cach2Cache == true &&  ((m_PndResp == false) || (m_PndResp == true && m_PendResp.reqCoreId != m_sharedCacheBusIfFIFO->m_fifo_id) || (m_ServQueueMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans)))) {
          m_GlobalReqFIFO.PopElement();
        }
      } 
      
      m_PndResp        = false;
      m_FcFsPndMemResp = false;
      // check service queue
      if (!m_GlobalReqFIFO.IsEmpty()) 
      {
        m_ServQueueMsg = m_GlobalReqFIFO.GetFrontElement();

        if (m_ServQueueMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans) 
        {
          m_respCoreCnt = m_ServQueueMsg.wbCoreId;
          m_PndResp = FcFsWriteBackCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr, true, m_PendResp);
          if (m_PndResp == false) 
          {
            m_TimeOut = m_TimeOut + 1;
            if (m_TimeOut == 5) 
            {
              m_TimeOut = 0;
              m_GlobalReqFIFO.PopElement();
            }
          }
          else {
              m_TimeOut = 0;
          }
        }
        else {
          m_respCoreCnt = m_ServQueueMsg.reqCoreId;
          m_FcFsPndMemResp = FcFsMemCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr);
          
          if (m_FcFsPndMemResp == false) {
            for (int i = 0; i < m_cpuCore && m_PndResp == false; i++) {
              m_respCoreCnt = i;
              m_PndResp = FcFsWriteBackCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr, true, m_PendResp);
            }
          }  
        }
      }
      // wait Resp-TDM response slot
      if (m_PndResp == true || m_FcFsPndMemResp == true) {
        Simulator::Schedule(NanoSeconds(m_dt*m_respclks), &BusArbiter::RespStep, Ptr<BusArbiter > (this));
      }
      else {// wait one clock cycle and check again
          Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::RespStep, Ptr<BusArbiter > (this));
      }
    }//void BusArbiter::PISCOT_MSI_FcFsResBus() 

    void BusArbiter::RR_RT_ReqBus(){      /* Request Bus Arbiter for the RT Controller */
      BusIfFIFO::BusReqMsg txResp;
      bool next_arb_level = true;
      // insert message on the request bus (broadcast)
      // in the slot, if there was a pending request chosen before, now its time to broadcast. and finally advance the TDM
      if (m_PndReq) {
        m_PndReq = CheckPendingReq (m_reqCoreCnt, m_ReqBusMsg, false);
        m_PndReq = false;
        // insert message on request bus
        InsertOnReqBus (m_ReqBusMsg);
        // insert message into service queue
        if (m_ReqBusMsg.NoGetMResp == false)                                       /// what is NoGetMResp?
        { 
          m_GlobalReqFIFO.InsertElement(m_ReqBusMsg);
        }
      }
      // Now, the chosen REQ is issued. We should check for the next transaction (according to the RR)
      // either if there was no pending requst or it was broadcasted, we should check for the next candidate
      // First check if the RR candidate has anything in its queue
      // Then advance RR order - if didn't find anything, advance until you find something  
      bool temp_PndReq = false;
      int pendingQueueSize = m_GlobalReqFIFO.GetQueueSize();
      /********************* First Level of Arbitration  ************************/
      for(unsigned int RR_iterator=0; RR_iterator < Global_RR_Queue.size() && m_PndReq == false; RR_iterator++) {
        // take the order based on the RR queue
        if(Global_RR_Queue.size() != 0) { m_reqCoreCnt = Order.at(RR_iterator)};				
        // check if the current candidate has a request in its local buffer
        temp_PndReq = CheckPendingReq(m_reqCoreCnt, m_ReqBusMsg, true);      
        m_PndReq    = temp_PndReq;
        // check if that core has a pending request in the service queue
        for (int j = 0; (j < pendingQueueSize && temp_PndReq == true) ;j++) {
          txResp = m_GlobalReqFIFO.GetFrontElement();
          m_GlobalReqFIFO.PopElement();
          if (m_reqCoreCnt == txResp.reqCoreId) { /*************** NEEDS MODIFICATION SINCE NOW WE HAVE MORE BANKS! HENCE, WE SHOULD CHECK IF THAT RESPONSE IS FOR THAT SPECIFIC BANK OR NOT? CORRECT? ***************/
            m_PndReq = false;
          }
          m_GlobalReqFIFO.InsertElement(txResp);
        }
      }
      if(m_PndReq) {next_arb_level = fasle;} /* if a msg is picked at first level, there is no need for the second level of arbiteration. */
      /********************* Second Level of Arbitration  ************************/
      if(next_arb_level)  {
        bool temp_PndReq = false;        
        for(unsigned int RR_iterator=0; RR_iterator < Global_RR_Queue.size() && m_PndReq == false; RR_iterator++) 
        {
          // take the order based on the RR queue
          if(Global_RR_Queue.size() != 0) { m_reqCoreCnt = Order.at(RR_iterator)};	
          temp_PndReq = CheckPendingReqNonOldest(m_reqCoreCnt, m_ReqBusMsg, true);  
          m_PndReq    = temp_PndReq;
          // check if that core has a pending request in the service queue
          for (int j = 0; (j < pendingQueueSize && temp_PndReq == true) ;j++) 
          {
            txResp = m_GlobalReqFIFO.GetFrontElement();
            m_GlobalReqFIFO.PopElement();
            if (m_reqCoreCnt == txResp.reqCoreId) { /*************** NEEDS MODIFICATION SINCE NOW WE HAVE MORE BANKS! HENCE, WE SHOULD CHECK IF THAT RESPONSE IS FOR THAT SPECIFIC BANK OR NOT? CORRECT? ***************/
              m_PndReq = false;
            }
            m_GlobalReqFIFO.InsertElement(txResp);
          }      
        }
      }
      // wait one TDM Request slot, if there is any request
      if (m_PndReq) {
        Simulator::Schedule(NanoSeconds(m_dt*m_reqclks), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
      else {// wait one clock cycle and check again
        Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
      }
    }

    void BusArbiter::RR_RT_RespBus(){     /* Response Bus Arbiter for the RT Controller */
      if (m_PndResp || m_FcFsPndMemResp) 
      {
        m_FcFsPndMemResp = (m_FcFsPndMemResp == true) ? FcFsMemCheckInsert       (m_respCoreCnt, m_ServQueueMsg.addr, false) :
                                                        FcFsWriteBackCheckInsert (m_respCoreCnt, m_ServQueueMsg.addr, false, m_PendResp);
        if ((m_cach2Cache == false &&  ((m_PndResp == false) || (m_ServQueueMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans))) 
            || (m_cach2Cache == true &&  ((m_PndResp == false) || (m_PndResp == true && m_PendResp.reqCoreId != m_sharedCacheBusIfFIFO->m_fifo_id) || (m_ServQueueMsg.cohrMsgId == SNOOPPrivCohTrans::PutMTrans)))) {
          m_GlobalReqFIFO.PopElement();
        }
      }
    }



    void BusArbiter::init() {
        BusArbDecode();
        Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::ReqStep, Ptr<BusArbiter > (this));
        Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::RespStep, Ptr<BusArbiter > (this));
        Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::Step, Ptr<BusArbiter > (this));
        Simulator::Schedule(NanoSeconds(m_clkSkew), &BusArbiter::L2CohrStep, Ptr<BusArbiter > (this));
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::L2CohrMsgHandle () {
       BusIfFIFO::BusReqMsg txreqMsg;
      if (!m_sharedCacheBusIfFIFO->m_txMsgFIFO.IsEmpty()) {
        txreqMsg = m_sharedCacheBusIfFIFO->m_txMsgFIFO.GetFrontElement();
        if (txreqMsg.cohrMsgId == SNOOPPrivCohTrans::ExclTrans) {
          SendMemCohrMsg(txreqMsg,false);
        }
        else { 
          SendMemCohrMsg(txreqMsg,true);
        }
        m_sharedCacheBusIfFIFO->m_txMsgFIFO.PopElement();
      }
      Simulator::Schedule(NanoSeconds(m_dt/4), &BusArbiter::L2CohrStep, Ptr<BusArbiter > (this));
    }
    
    void BusArbiter::ReqFncCall () {
        if (m_bus_arb == BusARBType::UNIFIED_TDM_ARB) {
          Unified_TDM_PMSI_Bus2();
        }
        else if (m_bus_arb == BusARBType::PISCOT_ARB) {
          if (m_cohProType == CohProtType::SNOOP_MSI ||
              m_cohProType == CohProtType::SNOOP_MESI||
              m_cohProType == CohProtType::SNOOP_MOESI) {
            PISCOT_OOO_MSI_TDM_ReqBus(); 
          }
          else if (m_cohProType == CohProtType::SNOOP_PMSI) {
            PMSI_OOO_TDM_ReqBus (); 
          }
          else {
            std::cout << "BusArbiter: Un-supported Coh. Protocol" << std::endl; 
            exit(0);         
          }
        }
        else if (m_bus_arb == BusARBType::FCFS_ARB) {
          if (m_cohProType == CohProtType::SNOOP_MSI || m_cohProType == CohProtType::SNOOP_PMSI || m_cohProType == CohProtType::SNOOP_MESI || m_cohProType == CohProtType::SNOOP_MOESI) {
            MSI_FcFsReqBus2 (); 
          }
          else {
            std::cout << "BusArbiter: Un-supported Coh. Protocol" << std::endl; 
            exit(0);         
          }
        }
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//            
    void BusArbiter::RespFncCall () {
        if (m_bus_arb == BusARBType::UNIFIED_TDM_ARB) {
            std::cout << "BusArbiter: [Info] Unified TDM Bus response scheduling is handled using Unified_TDM_PMSI_Bus function call" << std::endl;          
        }
        else if (m_bus_arb == BusARBType::PISCOT_ARB || 
                 m_bus_arb == BusARBType::FCFS_ARB) {
          if (m_cohProType == CohProtType::SNOOP_MSI || 
              m_cohProType == CohProtType::SNOOP_MESI||
              m_cohProType == CohProtType::SNOOP_MOESI) {
            if (m_bus_arb == BusARBType::PISCOT_ARB) {
              PISCOT_MSI_FcFsResBus();
            }
            else {
              MSI_FcFsRespBus(); 
            }
          }
          else if (m_cohProType == CohProtType::SNOOP_PMSI) {
            PMSI_FcFsRespBus();
          }
          else {
            std::cout << "BusArbiter: Un-supported Coh. Protocol" << std::endl; 
            exit(0);         
          }          
        }
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::BusArbDecode() {
      if (m_bus_arch == "unified") {
        if (m_cohProType != CohProtType::SNOOP_PMSI) {
          exit(0);
        }
        
        if (m_bus_arbiter == "PMSI" || (m_bus_arbiter == "CUSTOM" && m_reqbus_arb == "TDM" && m_respbus_arb == "TDM")) {
          m_bus_arb = BusARBType::UNIFIED_TDM_ARB;
        }
        else {
          exit(0);      
        }
        
        if (m_cach2Cache == true && m_bus_arbiter == "PMSI") {
            exit(0);
        }          
      }
      else if (m_bus_arch == "split") {
        if (m_bus_arbiter == "PISCOT") {
          m_bus_arb = BusARBType::PISCOT_ARB;
        }
        else if (m_bus_arbiter == "FCFS") {
          m_bus_arb = BusARBType::FCFS_ARB;        
        }
        else {
          exit(0);              
        }
        
        if (!(m_cohProType == CohProtType::SNOOP_PMSI || m_cohProType == CohProtType::SNOOP_MSI || m_cohProType == CohProtType::SNOOP_MESI || m_cohProType == CohProtType::SNOOP_MOESI)) {
          exit(0);        
        }      
      }
      
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//     
    /**
    * Call request/response bus functions
    * These function is called each interval dt
    */

    void BusArbiter::ReqStep(Ptr<BusArbiter> busArbiter) {
        busArbiter->ReqFncCall();
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//    
    void BusArbiter::RespStep(Ptr<BusArbiter> busArbiter) {
          busArbiter->RespFncCall();
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^//
    void BusArbiter::L2CohrStep(Ptr<BusArbiter> busArbiter) {
        busArbiter->L2CohrMsgHandle();
    }
        
    // Schedule the next run
    void BusArbiter::Step(Ptr<BusArbiter> busArbiter) {
      busArbiter->CycleAdvance();
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
    void BusArbiter::CycleAdvance () {
      if (m_stallDetectionEnable) {
        m_stall_cnt = (m_PndReq) ? 0 : (m_stall_cnt+1);
     
        if (m_stall_cnt >= Stall_CNT_LIMIT) {
          exit(0);
        }
      }
      
      m_arbiCycle++;
      Simulator::Schedule(NanoSeconds(m_dt), &BusArbiter::Step, Ptr<BusArbiter > (this));
      
    }
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^REWIEWD^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^// 
}
