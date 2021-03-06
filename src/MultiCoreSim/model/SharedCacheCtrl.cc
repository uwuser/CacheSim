/*
 * File  :      SharedCacheCtrl.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 20, 2020
 */

#include "SharedCacheCtrl.h"

namespace ns3 {
    TypeId SharedCacheCtrl::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::SharedCacheCtrl")
               .SetParent<Object > ();
        return tid;
    }

    SharedCacheCtrl::SharedCacheCtrl(uint32_t cachLines, 
                     Ptr<BusIfFIFO> assoicateBusIfFIFO,
                     Ptr<DRAMIfFIFO> associatedDRAMBusIfFifo) {
        // default
        m_cacheType        = 0;
        m_cacheSize        = 64*256;
        m_cacheBlkSize     = 64;
        m_victimCacheSize  = 64*4;
        m_nways            = 1;
        m_nsets            = 256;
        m_coreId           = 1;
        m_dt               = (1.0/1000000);
        m_clkSkew          = 0;
        m_cacheCycle       = 1;
        m_cache2Cache      = false;
        m_nPrivCores       = 4;
        m_replcPolicy      = ReplcPolicy::RANDOM;
        m_logFileGenEnable = false;
        m_cache            = CreateObject<GenericCache> (cachLines);
        m_busIfFIFO        = assoicateBusIfFIFO;
        m_dramBusIfFIFO    = associatedDRAMBusIfFifo;
        m_ownerCoreId      = new uint16_t[cachLines];
        m_PndWBFIFO.SetFifoDepth(30);
        m_DRAMOutStandingExclMsg.SetFifoDepth(30);
        m_l2CachePreloadFlag = false;
        m_dramSimEnable   = false;
        m_dramLatcy       = 100;
        m_dramOutstandReq = 6;
        m_dramModle       = "FIXEDLat";
        m_Nreqs           = 0;
        m_Nmiss           = 0;
    }

    // We don't do any dynamic allocations
    SharedCacheCtrl::~SharedCacheCtrl() {
    }

    void SharedCacheCtrl::SetCacheSize (uint32_t cacheSize) {
      m_cacheSize = cacheSize;
      m_cache->SetCacheSize(cacheSize);
    }

    uint32_t SharedCacheCtrl::GetCacheSize () {
      return m_cacheSize;
    }

    void SharedCacheCtrl::SetCacheBlkSize (uint32_t cacheBlkSize) {
      m_cacheBlkSize = cacheBlkSize;
      m_cache->SetCacheBlkSize(cacheBlkSize);
    }

    uint32_t SharedCacheCtrl::GetCacheBlkSize () {
      return m_cacheBlkSize;
    }

    void SharedCacheCtrl::SetCacheNways (uint32_t nways) {
      m_nways = nways;
      m_cache->SetCacheNways(nways);
    }

    uint32_t SharedCacheCtrl::GetCacheNways () {
      return m_nways;
    }

    void SharedCacheCtrl::SetCacheNsets (uint32_t nsets) {
      m_nsets = nsets;
      m_cache->SetCacheNsets(nsets);
    }

    uint32_t SharedCacheCtrl::GetCacheNsets () {
      return m_nsets;
    }

    void SharedCacheCtrl::SetCacheType (uint16_t cacheType) {
      m_cacheType = cacheType;
      m_cache->SetCacheType(cacheType);
    }

    void SharedCacheCtrl::SetVictCacheSize (uint32_t cacheSize) {
      m_victimCacheSize = cacheSize;
    }
    
    uint16_t SharedCacheCtrl::GetCacheType () {
      return m_cacheType;
    }
    
    void SharedCacheCtrl::SetReplcPolicy (ReplcPolicy replcPolicy) {
      m_replcPolicy = replcPolicy;
    }

    void SharedCacheCtrl::SetCoreId (int coreId) {
      m_coreId = coreId;
    }

    int SharedCacheCtrl::GetCoreId () {
      return m_coreId;
    }

    void SharedCacheCtrl::SetDt (double dt) {
      m_dt = dt;
    }

    int SharedCacheCtrl::GetDt () {
      return m_dt;
    }

    void SharedCacheCtrl::SetClkSkew (double clkSkew) {
       m_clkSkew = clkSkew;
    }

    void SharedCacheCtrl::SetCache2Cache (bool cache2Cache) {
       m_cache2Cache = cache2Cache;
    }

    void SharedCacheCtrl::SetBMsPath  (std::string bmsPath) {
      m_bmsPath = bmsPath;
    }

    void SharedCacheCtrl::SetNumPrivCore (int nPrivCores) {
      m_nPrivCores = nPrivCores;
    }

    void SharedCacheCtrl::SetLogFileGenEnable (bool logFileGenEnable ) {
      m_logFileGenEnable = logFileGenEnable;
    }

    void SharedCacheCtrl::SetDramSimEnable (bool dramSim_en ) {
      m_dramSimEnable = dramSim_en;
    }
    
    void SharedCacheCtrl::SetDramFxdLatcy (uint32_t dramLatcy ) {
      m_dramLatcy = dramLatcy;
    }
    
    void SharedCacheCtrl::SetDramModel (std::string dramModel ) {
      m_dramModle = dramModel;
    }
    
    void SharedCacheCtrl::SetDramOutstandReq (uint32_t dramOutstandReqs) {
      m_dramOutstandReq = dramOutstandReqs;
    }
    
    void SharedCacheCtrl::SetCachePreLoad (bool l2CachePreload) {
      m_l2CachePreloadFlag = l2CachePreload;
    }
    
    uint64_t SharedCacheCtrl::GetShareCacheMisses () {
      return m_Nmiss;
    }
    
    uint64_t SharedCacheCtrl::GetShareCacheNReqs () {
      return m_Nreqs;
    }
     
     SNOOPSharedCtrlEvent SharedCacheCtrl::ChkDRAMReqEvent (SNOOPSharedReqBusEvent busReqEvent) {
       SNOOPSharedCtrlEvent DRAMReqEvent;
       
       if (busReqEvent == SNOOPSharedReqBusEvent::GetM) {
         DRAMReqEvent = SNOOPSharedCtrlEvent::DRAMGetM;
       }
       else if (busReqEvent == SNOOPSharedReqBusEvent::GetS) {
         DRAMReqEvent = SNOOPSharedCtrlEvent::DRAMGetS;
       }
       else if (busReqEvent == SNOOPSharedReqBusEvent::Null) {
         DRAMReqEvent = SNOOPSharedCtrlEvent::Null;
       }
       else if (busReqEvent == SNOOPSharedReqBusEvent::Upg || 
                busReqEvent == SNOOPSharedReqBusEvent::OwnerPutM ||
                busReqEvent == SNOOPSharedReqBusEvent::OTherPutM ||
                busReqEvent == SNOOPSharedReqBusEvent::PutS ) {
         std::cout << "SharedMem: [Error] invalid Transaction for DRAM Request" << std::endl;
         exit(0);
       }
       else {
         std::cout << "SharedMem: [Error] uncovered ReqBus Event" << std::endl;
         exit(0);
       }
       return DRAMReqEvent;
     }
     
     SNOOPSharedReqBusEvent SharedCacheCtrl::ChkBusRxReqEvent  (BusIfFIFO::BusReqMsg &  busReqMsg) {  
       if (!m_busIfFIFO->m_rxMsgFIFO.IsEmpty()) {

         SNOOPSharedReqBusEvent reqBusEvent;
         busReqMsg = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();

         uint16_t wbCoreId   = busReqMsg.wbCoreId;
         uint32_t  busReqCacheWayIdx;
         uint16_t ownerCoreId;
         GenericCache::CacheLineInfo CacheLineInfo;
         VictimCache ::CacheLineInfo VictimCacheLineInfo;
         
         CacheLineInfo  = m_cache->GetCacheLineInfo(busReqMsg.addr);
         VictimCacheLineInfo = m_victimCache->GetCacheLineInfo(busReqMsg.addr); 
         
         if (CacheLineInfo.IsValid) {
           busReqCacheWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;             
           ownerCoreId = m_ownerCoreId[busReqCacheWayIdx];
         }
         else {
           ownerCoreId = m_victimOwnerCoreId[VictimCacheLineInfo.line_idx];
         }
           
         switch (busReqMsg.cohrMsgId) {
           case SNOOPPrivCohTrans::GetSTrans:
             reqBusEvent = SNOOPSharedReqBusEvent::GetS;
             break;
           case SNOOPPrivCohTrans::GetMTrans:
             reqBusEvent = SNOOPSharedReqBusEvent::GetM;
             break;
           case SNOOPPrivCohTrans::UpgTrans :
             reqBusEvent = SNOOPSharedReqBusEvent::Upg; 
             break;
           case SNOOPPrivCohTrans::PutMTrans:
             reqBusEvent = (wbCoreId == ownerCoreId) ? 
                            SNOOPSharedReqBusEvent::OwnerPutM : 
                            SNOOPSharedReqBusEvent::OTherPutM ;
             break;
           case SNOOPPrivCohTrans::PutSTrans:
             reqBusEvent = SNOOPSharedReqBusEvent::PutS;
             break;
           default: // Invalid Transaction
             if (m_logFileGenEnable) {
               std::cout << "SharedMem: [Error] invalid Transaction detected on the Bus" << std::endl;
             }
             exit(0);
           }
         return reqBusEvent;
       }
       else {// CoreNullEvent
         return SNOOPSharedReqBusEvent::Null;
       }
     }

     SNOOPSharedRespBusEvent SharedCacheCtrl::ChkBusRxRespEvent (BusIfFIFO::BusRespMsg & busRespMsg){

       if (!m_busIfFIFO->m_rxRespFIFO.IsEmpty()) {
         busRespMsg = m_busIfFIFO->m_rxRespFIFO.GetFrontElement();
         uint16_t respCoreId   = busRespMsg.respCoreId;
         return (respCoreId == m_coreId) ? SNOOPSharedRespBusEvent::OWnDataResp :
                                           SNOOPSharedRespBusEvent::OTherDataResp;
       }
       else {
         return SNOOPSharedRespBusEvent::NUll;
       }
     }

     void SharedCacheCtrl::UpdateSharedCache   (CacheField field, 
                                                uint64_t addr, 
                                                int state, 
                                                uint8_t * data,
                                                bool UpdateAccessCnt = false) {

       GenericCacheFrmt cacheLine;
       uint32_t LineWayIdx;
       if (FetchLine(addr, cacheLine, LineWayIdx)) {
       
         if (UpdateAccessCnt) {
           cacheLine.accessCounter = cacheLine.accessCounter + 1;
           cacheLine.accessCycle   = m_cacheCycle;
         }
         
         if (field == State) {
           cacheLine.state  = state;
           m_cache->WriteCacheLine(cacheLine, LineWayIdx);
         }
         else if (field == Line) {
           cacheLine.state  = state;
           cacheLine.tag    = m_cache->CpuAddrMap (addr).tag;
           for (int i = 0; i < 8; i++)
             cacheLine.data[i] = data[i];

           m_cache->WriteCacheLine(cacheLine, LineWayIdx);
         }
       }
       else {
         if (m_logFileGenEnable) {
           std::cout << "SharedMem: [Warning] Cannot find the block in shared cache!" << std::endl;
         }
         exit(0);
       }
     }

     bool SharedCacheCtrl::FetchLine (uint64_t addr, GenericCacheFrmt  & cacheLine, uint32_t & LineWayIdx) {
       GenericCacheMapFrmt addrMap   = m_cache->CpuAddrMap (addr);
       uint32_t setIdx = addrMap.idx_set;
       uint32_t nWays  = m_cache -> GetCacheNways();
       uint32_t setOfst = setIdx * nWays;

       for (uint32_t wayIdx = setOfst; wayIdx < setOfst+nWays;wayIdx++) {
         cacheLine    = m_cache->ReadCacheLine(wayIdx);
         LineWayIdx   = wayIdx;
         if (cacheLine.valid == true && cacheLine.tag == addrMap.tag) {
           return true;
         }
       }
       return false;
     }

     // execute write back or sent command
     bool SharedCacheCtrl::DoWriteBack (uint64_t addr, uint16_t wbCoreId, uint64_t msgId, double timestamp, SendDataType type = DataOnly) {
     
       BusIfFIFO::BusReqMsg tempBusReqMsg;
       BusIfFIFO::BusRespMsg  wbMsg;
       
       tempBusReqMsg.msgId        = msgId;
       tempBusReqMsg.reqCoreId    = wbCoreId;
       tempBusReqMsg.wbCoreId     = m_coreId;
       tempBusReqMsg.cohrMsgId    = SNOOPPrivCohTrans::ExclTrans;
       tempBusReqMsg.addr         = addr;
       tempBusReqMsg.timestamp    = timestamp;
       tempBusReqMsg.cycle        = m_cacheCycle;
       tempBusReqMsg.cohrMsgId    = (type == CoreInv) ? SNOOPPrivCohTrans::InvTrans : SNOOPPrivCohTrans::ExclTrans;
       bool TxReqMsgInsertFlag    = false;
       bool TxRespMsgInsertFlag   = false;
       
       if (type == CoreInv || type == DataPlsExcl || type == ExclOnly) {
         if (!m_busIfFIFO->m_txMsgFIFO.IsFull()) {
           m_busIfFIFO->m_txMsgFIFO.InsertElement(tempBusReqMsg);
           TxReqMsgInsertFlag = true;
         }
       }
       else {
         TxReqMsgInsertFlag = true;
       }
       
       if (type == DataOnly || type == DataPlsExcl) {
         wbMsg.reqCoreId    = wbCoreId;
         wbMsg.respCoreId   = m_coreId;
         wbMsg.addr         = addr;
         wbMsg.msgId        = msgId;
         wbMsg.timestamp    = timestamp;
         wbMsg.dualTrans    = false;
         wbMsg.cycle        = m_cacheCycle;
         
         for (int i = 0; i < 8; i++)
           wbMsg.data[i] = 0;

         if (!m_busIfFIFO->m_txRespFIFO.IsFull()) {
           m_busIfFIFO->m_txRespFIFO.InsertElement(wbMsg);
           TxRespMsgInsertFlag = true;
         }
         
         if (m_logFileGenEnable) {
           std::cout << "Sent data to core = " << wbMsg.reqCoreId << ", addr " << addr << std::endl;
         }
         
       }
       else {
         TxRespMsgInsertFlag = true;
       }
       
       return (TxReqMsgInsertFlag && TxRespMsgInsertFlag);    
     }

     bool SharedCacheCtrl::PushMsgInBusTxFIFO  (uint64_t       msgId, 
                                                uint16_t       reqCoreId, 
                                                uint16_t       wbCoreId, 
                                                uint64_t       addr) {
       if (!m_PndWBFIFO.IsFull()) {
         BusIfFIFO::BusReqMsg tempBusReqMsg;
         tempBusReqMsg.msgId        = msgId;
         tempBusReqMsg.reqCoreId    = reqCoreId;
         tempBusReqMsg.wbCoreId     = wbCoreId;
         tempBusReqMsg.addr         = addr;
         tempBusReqMsg.timestamp    = m_cacheCycle*m_dt;
         // push message into BusTxMsg FIFO
         m_PndWBFIFO.InsertElement(tempBusReqMsg);
         return true;
       }
       else {
         if (m_logFileGenEnable) {
           std::cout << "SharedMem: [Warning] Cannot insert the Msg in BusTxMsg FIFO, FIFO is Full!" << std::endl;
         }
         return false;
       }
     }

    // send memory request to DRAM
    bool SharedCacheCtrl::SendDRAMReq (uint64_t msgId, uint64_t addr, DRAMIfFIFO::DRAM_REQ type) {
      DRAMIfFIFO::DRAMReqMsg dramReqMsg;
      dramReqMsg.msgId       = msgId;
      dramReqMsg.addr        = addr;
      dramReqMsg.type        = type;
      dramReqMsg.reqAgentId  = m_coreId;
      dramReqMsg.cycle       = m_cacheCycle;

      bool InsertFlag = false;
      if (!m_dramBusIfFIFO->m_txReqFIFO.IsFull()) {
        m_dramBusIfFIFO->m_txReqFIFO.InsertElement(dramReqMsg);
        InsertFlag = true;
      }
      return InsertFlag;
    }
    
     void SharedCacheCtrl::SendPendingReqData  (GenericCacheMapFrmt recvTrans ) {
       if (!m_PndWBFIFO.IsEmpty()) {
          BusIfFIFO::BusReqMsg pendingWbMsg ;
          GenericCacheMapFrmt  pendingWbAddrMap;
          int pendingQueueSize = m_PndWBFIFO.GetQueueSize();
          for (int i = 0; i < pendingQueueSize ;i++) {
            pendingWbMsg = m_PndWBFIFO.GetFrontElement();
            pendingWbAddrMap = m_cache->CpuAddrMap (pendingWbMsg.addr);
            m_PndWBFIFO.PopElement();
            if (recvTrans.idx_set == pendingWbAddrMap.idx_set &&
                recvTrans.tag == pendingWbAddrMap.tag) {
              if (!DoWriteBack (pendingWbMsg.addr,pendingWbMsg.reqCoreId,pendingWbMsg.msgId,pendingWbMsg.timestamp)) {
                if (m_logFileGenEnable){
                  std::cout << "SharedMem: Cannot Send Data On the Bus !!!!" << std::endl;
                }
                exit(0);
              }
            }
            else {
              m_PndWBFIFO.InsertElement(pendingWbMsg);
            }
          }
        } 
     }  
      
     void SharedCacheCtrl::VictimCacheLineEvict (uint32_t victimWayIdx) {
       uint64_t              ReplcAddr;
       GenericCacheFrmt      ReplcCacheLine ;
       SNOOPSharedOwnerState CacheLineOwner;
       
       ReplcCacheLine = m_victimCache->ReadCacheLine(victimWayIdx);
       ReplcAddr      = ReplcCacheLine.tag;                     
                   
       if (ReplcCacheLine.valid) { 
         m_CurrEventList.CtrlEvent = SNOOPSharedCtrlEvent::Replacement;
         CacheLineOwner = (m_victimOwnerCoreId[victimWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore ;

         CohProtocolFSMProcessing (SNOOPSharedEventType::CacheCtrl, ReplcCacheLine.state, CacheLineOwner);
                 
         if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite || 
           m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInv) { 
           if (!DoWriteBack (ReplcAddr,m_CurrEventMsg.busReqMsg.reqCoreId,m_CurrEventMsg.busReqMsg.msgId, m_cacheCycle*m_dt, CoreInv)) {
             std::cout << "SharedMem: Cannot send Inv() on request Bus !!!!" << std::endl;
             exit(0);
           }
         }
                     
         if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite || 
           m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueDRAMWrite) {
           if (!SendDRAMReq (m_CurrEventMsg.busReqMsg.msgId, ReplcAddr, DRAMIfFIFO::DRAM_REQ::DRAM_WRITE)) {
             std::cout << "SharedMem: Error: Cannot Direct Write Request to DRAM " << std::endl;
             exit(0);
           }
         }
            
         ReplcCacheLine.evicted   = false;
         if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite || 
           m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::IssueDRAMWrite) {
           ReplcCacheLine.evicted   = true;
         }
         ReplcCacheLine.state   = m_CurrEventNextState.CtrlEventState;
         m_victimCache->WriteCacheLine(victimWayIdx, ReplcCacheLine);    
         
         if (m_CurrEventOwnerNextState.CtrlOwnerState == SNOOPSharedOwnerState::SharedMem) {
            m_victimOwnerCoreId[victimWayIdx] = m_coreId;
         }          
       } // if (!ReplcCacheLine.valid) { 
     }
     

     void SharedCacheCtrl::SendExclRespEarly () {
       bool OutStandingExclFlag = false;  // default
       int ExclQueueSize;
       uint64_t OutstandingExclAddr;
       if ((m_cohrProt == CohProtType::SNOOP_MESI || 
            m_cohrProt == CohProtType::SNOOP_MOESI)) {
                   
         if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS ||
             m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
           ExclQueueSize = m_DRAMOutStandingExclMsg.GetQueueSize();
           for (int j = 0; j < ExclQueueSize; j++ ) {
             OutstandingExclAddr = m_DRAMOutStandingExclMsg.GetFrontElement();
             m_DRAMOutStandingExclMsg.PopElement();
             if ((OutstandingExclAddr >> (int) log2(m_cacheBlkSize))  == 
                 (m_CurrEventMsg.busReqMsg.addr >> (int) log2(m_cacheBlkSize)))  {
               OutStandingExclFlag = true;
             }
             m_DRAMOutStandingExclMsg.InsertElement(OutstandingExclAddr);  
           }
                   
           if (OutStandingExclFlag == false) {
             m_DRAMOutStandingExclMsg.InsertElement(m_CurrEventMsg.busReqMsg.addr);
             if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS) {
               if (!DoWriteBack (m_CurrEventMsg.busReqMsg.addr,m_CurrEventMsg.busReqMsg.reqCoreId,m_CurrEventMsg.busReqMsg.msgId, m_cacheCycle*m_dt, ExclOnly)) {
                 std::cout << "SharedMem: Cannot Send Exclusive Msg on the bus !!!!" << std::endl;
                 exit(0);
               }
             }
           }       
         }
       }
     }
          
     bool SharedCacheCtrl::RemoveExclRespAddr () {
          int ExclQueueSize = m_DRAMOutStandingExclMsg.GetQueueSize();
          bool OutStandingExclFlag = false;  // default
          uint64_t OutstandingExclAddr;
          for (int j = 0; j < ExclQueueSize; j++ ) {
            OutstandingExclAddr = m_DRAMOutStandingExclMsg.GetFrontElement();
            m_DRAMOutStandingExclMsg.PopElement();
            if ((OutstandingExclAddr >> (int) log2(m_cacheBlkSize))  == 
                (m_CurrEventMsg.busReqMsg.addr >> (int) log2(m_cacheBlkSize)) && OutStandingExclFlag == false)  {
              OutStandingExclFlag = true;
            }
            else {
              m_DRAMOutStandingExclMsg.InsertElement(OutstandingExclAddr);  // dequeue
            }
          }
          return OutStandingExclFlag;
     }
                               
     // This function does most of the functionality.
     void SharedCacheCtrl::CacheCtrlMain () {
     
       GenericCacheFrmt            TempCacheline, 
                                   ReplcCacheLine;
                                   
       uint32_t  SclWayIdx;
              

       GenericCache::CacheLineInfo CacheLineInfo;
       VictimCache ::CacheLineInfo VictimCacheLineInfo;
       
       SNOOPSharedOwnerState       CacheLineOwner;
       int QueueSize;
       bool NewFetchDone;
       bool ProcFromVictimCache;

       QueueSize = m_dramBusIfFIFO->m_rxRespFIFO.GetQueueSize();
       NewFetchDone = false; 
       DRAMIfFIFO::DRAMRespMsg dramBusRespMsg;
                        
       for (int i = 0; i < QueueSize; i++ ) { 
         dramBusRespMsg = m_dramBusIfFIFO->m_rxRespFIFO.GetFrontElement();
         m_dramBusIfFIFO->m_rxRespFIFO.PopElement();
         
         if (NewFetchDone == false) {
           CacheLineInfo = m_cache->GetCacheLineInfo(dramBusRespMsg.addr); 
           if (CacheLineInfo.IsValid == true) { 
             NewFetchDone = true;
             m_CurrEventList.busRespEvent = SNOOPSharedRespBusEvent::DRAMDataResp;
             CohProtocolFSMProcessing (SNOOPSharedEventType::RespBus, CacheLineInfo.state, SNOOPSharedOwnerState::SharedMem);
             
             if (m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::SharedFault) {
               std::cout << "SharedMem: DRAM DataResp occur in illegal state!" << std::endl; 
               exit(0);
             }
             else {
               TempCacheline = m_cache->ReadCacheLine(CacheLineInfo.set_idx, CacheLineInfo.way_idx);
               TempCacheline.state = m_CurrEventNextState.busRespEventState;
               m_cache->WriteCacheLine (CacheLineInfo.set_idx, CacheLineInfo.way_idx, TempCacheline);
               
               SclWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;
               if (m_CurrEventOwnerNextState.busRespOwnerState == SNOOPSharedOwnerState::SharedMem) {
                 m_ownerCoreId[SclWayIdx] = m_coreId;
               }
               else {
                 std::cout << "SharedCache: DRAM DataResp illegal Owner Set!" << std::endl; 
                 exit(0);
               }      
             }
           }
           else { 
             m_dramBusIfFIFO->m_rxRespFIFO.InsertElement(dramBusRespMsg);
           }
         }
         else {
           m_dramBusIfFIFO->m_rxRespFIFO.InsertElement(dramBusRespMsg);
         }
       } // for (int i = 0; i < QueueSize; i++ ) { 
       
       m_CurrEventList.busRespEvent = ChkBusRxRespEvent (m_CurrEventMsg.busRespMsg); 
       GenericCacheMapFrmt respAddrMap   = m_cache->CpuAddrMap (m_CurrEventMsg.busRespMsg.addr); 
       ProcFromVictimCache = false;
       
       if (m_CurrEventList.busRespEvent != SNOOPSharedRespBusEvent::NUll) {
         CacheLineInfo       = m_cache->GetCacheLineInfo(m_CurrEventMsg.busRespMsg.addr);
         VictimCacheLineInfo = m_victimCache->GetCacheLineInfo(m_CurrEventMsg.busRespMsg.addr); 
       
         if (CacheLineInfo.IsValid == false) {
           if (VictimCacheLineInfo.IsValid == false) { 
             std::cout << "SharedMem: Error Response message is not in L2 Cache or Victim Cache" << std::endl;
             exit(0);
           }
           else {
             ProcFromVictimCache = true;
             CacheLineOwner = (m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
             CohProtocolFSMProcessing (SNOOPSharedEventType::RespBus, VictimCacheLineInfo.state, CacheLineOwner); // process RespBus event
             
             if (m_logFileGenEnable){ // debug message
               std::cout << "SharedMem: Process L2 response message from Victim Cache, wayIdx = " << VictimCacheLineInfo.line_idx << std::endl;
             }
           }
         }
         else {
           SclWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;
           CacheLineOwner = (m_ownerCoreId[SclWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
           CohProtocolFSMProcessing (SNOOPSharedEventType::RespBus, CacheLineInfo.state, CacheLineOwner);
         }
         
         if (m_logFileGenEnable){ // debug message
           if (m_CurrEventAction.busRespAction != SNOOPSharedCtrlAction::SharedNullAck) {
             std::cout << "\nSharedMem: " << m_coreId << " has Msg in the RxResp Bus" << std::endl;
             std::cout << "\t\t BusEventName        = " << PrintSharedRespBusEventName(m_CurrEventList.busRespEvent) << std::endl;
             std::cout << "\t\t ReqCoreId           = " << m_CurrEventMsg.busRespMsg.reqCoreId << " RespCoreId = " << m_CurrEventMsg.busRespMsg.respCoreId 
                       << " Resp Addr  = " << m_CurrEventMsg.busRespMsg.addr << " CacheLine = " << respAddrMap.idx_set << std::endl;
             std::cout << "\t\t CacheLine CurrState = " << PrintSharedStateName(m_CurrEventCurrState.busRespEventState) << std::endl;
             std::cout << "\t\t CacheLine NextState = " << PrintSharedStateName(m_CurrEventNextState.busRespEventState) << std::endl;
             std::cout << "\t\t Ctrl ReqAction      = " << PrintSharedActionName(m_CurrEventAction.busRespAction) << std::endl;
           }
         } 
      
         if (m_CurrEventAction.busRespAction != SNOOPSharedCtrlAction::SharedNullAck) {
           if (m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::StoreData || 
               m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::StoreDataOnly) {
             m_busIfFIFO->m_rxRespFIFO.PopElement();
             if (ProcFromVictimCache) {
               TempCacheline = m_victimCache->ReadCacheLine(VictimCacheLineInfo.line_idx); 
               TempCacheline.state = m_CurrEventNextState.busRespEventState;
               m_victimCache->WriteCacheLine(VictimCacheLineInfo.line_idx, TempCacheline);
             }
             else {
               UpdateSharedCache (Line, m_CurrEventMsg.busRespMsg.addr, m_CurrEventNextState.busRespEventState, m_CurrEventMsg.busRespMsg.data);
             }
        
             if (m_CurrEventAction.busRespAction != SNOOPSharedCtrlAction::StoreDataOnly) {
               SendPendingReqData(respAddrMap);
             }
           }
           else if (m_CurrEventAction.busRespAction == SNOOPSharedCtrlAction::SharedNoAck) {
             m_busIfFIFO->m_rxRespFIFO.PopElement();
           }
           else { 
             if (m_logFileGenEnable){
               std::cout << "SharedCache: DataResp occur in illegal state!" << std::endl; 
             }   
             exit(0);     
           }

           m_CurrEventAction.busRespAction = SNOOPSharedCtrlAction::SharedProcessedAck;
         }
       }  // if (m_CurrEventList.busRespEvent != SNOOPSharedRespBusEvent::NUll) {

       bool EvictDoneFlag = false;
       bool OutStandingExclFlag;
       GenericCacheFrmt busReqCacheLine;
       GenericCacheMapFrmt reqbusAddrMap;
                          
       BusIfFIFO::BusReqMsg        BusReqMsgTemp = {};
       SNOOPSharedReqBusEvent      BusReqEventTemp;
                                   
       int emptyWay,
           replcWayIdx,
           victimWayIdx;
           
       uint16_t tmpVictimOwnerCoreId;
       
       SendDataType sendType;
     
       
       SNOOPSharedCtrlEvent DRAMReqEvent;
       
       bool ItrSkipSwappingChk;
       NewFetchDone = false;
       ItrSkipSwappingChk = false;
       QueueSize = m_busIfFIFO->m_rxMsgFIFO.GetQueueSize();
       for (int i = 0; i < QueueSize; i++ ) {
         m_CurrEventList.busReqEvent  = ChkBusRxReqEvent  (m_CurrEventMsg.busReqMsg );
         m_busIfFIFO->m_rxMsgFIFO.PopElement();
         
         if (NewFetchDone == false) {
           ProcFromVictimCache = false;
           BusReqMsgTemp   = m_CurrEventMsg.busReqMsg;
           BusReqEventTemp = m_CurrEventList.busReqEvent;
           CacheLineInfo  = m_cache->GetCacheLineInfo(m_CurrEventMsg.busReqMsg.addr);
           VictimCacheLineInfo = m_victimCache->GetCacheLineInfo(m_CurrEventMsg.busReqMsg.addr); 
           reqbusAddrMap         = m_cache->CpuAddrMap (m_CurrEventMsg.busReqMsg.addr);
           
           if (CacheLineInfo.IsValid == false) { 
             if (VictimCacheLineInfo.IsValid == false) { 
               DRAMReqEvent = ChkDRAMReqEvent (m_CurrEventList.busReqEvent);
               m_CurrEventList.CtrlEvent = DRAMReqEvent; 
               SendExclRespEarly();

               TempCacheline.valid   = true;
               TempCacheline.evicted = false;
               TempCacheline.state   = GetWaitDRAMRespState();
               TempCacheline.mru_bit = false;
               TempCacheline.tag     = reqbusAddrMap.tag;
               TempCacheline.insertCycle = m_cacheCycle;
               TempCacheline.accessCounter = 0;
               TempCacheline.accessCycle   = m_cacheCycle;
               
               if (CacheLineInfo.IsSetFull == false) { 
                 emptyWay = m_cache->GetEmptyCacheLine(CacheLineInfo.set_idx);
                 if (emptyWay == -1) {
                   std::cout << "SharedMem: Fatal non-empty Condition" << std::endl;
                   exit(0);
                 }
                 else {
                   m_cache->WriteCacheLine (CacheLineInfo.set_idx, emptyWay, TempCacheline);
                   EvictDoneFlag = true; 
                 }
               }
               else { 
                 if (!VictimCacheLineInfo.IsSetFull) { 
                   replcWayIdx     = m_cache->GetReplacementLine(CacheLineInfo.set_idx, m_replcPolicy);  
                   ReplcCacheLine  = m_cache->ReadCacheLine(CacheLineInfo.set_idx, replcWayIdx);
                   SclWayIdx       = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + replcWayIdx;
                   
                   CacheLineOwner = (m_ownerCoreId[SclWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
                   CohProtocolFSMProcessing (SNOOPSharedEventType::CacheCtrl, ReplcCacheLine.state, CacheLineOwner);
                    
                   if (m_CurrEventAction.CtrlAction == SNOOPSharedCtrlAction::SendVictimCache) {                   
                     victimWayIdx   = m_victimCache->GetNextWritePtr();
                     m_victimOwnerCoreId[victimWayIdx] = m_ownerCoreId[SclWayIdx];
                     ReplcCacheLine.tag = m_cache->CpuPhyAddr (CacheLineInfo.set_idx, replcWayIdx); 
                     ReplcCacheLine.insertCycle = m_cacheCycle;
                     m_victimCache->WriteCacheLine(victimWayIdx,ReplcCacheLine);
                     
                     m_cache->WriteCacheLine (CacheLineInfo.set_idx, replcWayIdx, TempCacheline);
                     EvictDoneFlag = true;    
                   }
                   else {
                     m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
                   }   
                 } // if (!VictimCacheLineInfo.IsSetFull) { 
                 else { 
                   m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
                 }
               } 
             }
             else { 
               tmpVictimOwnerCoreId = m_victimOwnerCoreId[VictimCacheLineInfo.line_idx];
               CacheLineOwner = (tmpVictimOwnerCoreId == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore;
               TempCacheline = m_victimCache->ReadCacheLine(VictimCacheLineInfo.line_idx); 
               CohProtocolFSMProcessing (SNOOPSharedEventType::ReqBus, VictimCacheLineInfo.state, CacheLineOwner); 
               if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedNoAck) { 
                 ProcFromVictimCache = true;
               }
               else { 
                 replcWayIdx     = m_cache->GetReplacementLine(CacheLineInfo.set_idx, m_replcPolicy);
                 ReplcCacheLine  = m_cache->ReadCacheLine(CacheLineInfo.set_idx, replcWayIdx);
                 SclWayIdx       = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + replcWayIdx;
                 
                 m_CurrEventList.CtrlEvent = SNOOPSharedCtrlEvent::VictCacheSwap;
                 CohProtocolFSMProcessing (SNOOPSharedEventType::CacheCtrl, ReplcCacheLine.state, CacheLineOwner);
                   
                 if (m_CurrEventAction.CtrlAction != SNOOPSharedCtrlAction::SWAPPING || ItrSkipSwappingChk == true) { 
                   ItrSkipSwappingChk = true;
                   CohProtocolFSMProcessing (SNOOPSharedEventType::ReqBus, VictimCacheLineInfo.state, CacheLineOwner); 
                   if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveRespStall ||
                       m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendDataExclusive || 
                       m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
                     SendExclRespEarly();
                   }
                   m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;              
                 }
                 else { 
             
                   TempCacheline.tag           = reqbusAddrMap.tag;  
                   TempCacheline.insertCycle   = m_cacheCycle;
                   TempCacheline.accessCounter = 0;
                   TempCacheline.accessCycle   = m_cacheCycle;
                   TempCacheline.evicted       = false;
                   
                   ReplcCacheLine.tag = m_cache->CpuPhyAddr (CacheLineInfo.set_idx, replcWayIdx);
                   ReplcCacheLine.insertCycle = m_cacheCycle; 
                   m_cache->WriteCacheLine (CacheLineInfo.set_idx, replcWayIdx, TempCacheline);
                   m_victimCache->WriteCacheLine(VictimCacheLineInfo.line_idx, ReplcCacheLine);
                   m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] = m_ownerCoreId[SclWayIdx];
                   
                   m_ownerCoreId[SclWayIdx] = tmpVictimOwnerCoreId;
                   CacheLineInfo  = m_cache->GetCacheLineInfo(m_CurrEventMsg.busReqMsg.addr);
               
                   if (CacheLineInfo.IsValid == false) {
                     std::cout << "SharedMem: Fatal Error after swapping victim cache with L2 Cache" << std::endl;
                     exit(0);               
                   }
                 }
               }
             }
           } // if (CacheLineInfo.IsValid == false) {
           
           if (CacheLineInfo.IsValid) {
             SclWayIdx = m_cache -> GetCacheNways() * CacheLineInfo.set_idx + CacheLineInfo.way_idx;             
             CacheLineOwner = (m_ownerCoreId[SclWayIdx] == m_coreId) ? SNOOPSharedOwnerState::SharedMem : SNOOPSharedOwnerState::OtherCore ;
             CohProtocolFSMProcessing (SNOOPSharedEventType::ReqBus, CacheLineInfo.state, CacheLineOwner); // process ReqBus event
             if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveRespStall) {
               m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
               SendExclRespEarly();
             }
           } 
                   
           if (EvictDoneFlag) {
             m_Nmiss++;
             m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedStall;
               if (!SendDRAMReq (m_CurrEventMsg.busReqMsg.msgId, m_CurrEventMsg.busReqMsg.addr, DRAMIfFIFO::DRAM_REQ::DRAM_READ)) {
                 std::cout << "SharedMem: Error: Cannot Direct Request to DRAM " << std::endl;
                 exit(0);
               }
           }
           
           if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedStall) {
             m_busIfFIFO->m_rxMsgFIFO.InsertElement(m_CurrEventMsg.busReqMsg);
           }
           
           if (EvictDoneFlag || m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedStall) {
             NewFetchDone = true;
           }   
         }
         else {
           m_busIfFIFO->m_rxMsgFIFO.InsertElement(m_CurrEventMsg.busReqMsg);
         }
       }  // for (int i = 0; i < QueueSize; i++ ) {

       m_CurrEventMsg.busReqMsg = BusReqMsgTemp;
       m_CurrEventList.busReqEvent = BusReqEventTemp;
       reqbusAddrMap   = m_cache->CpuAddrMap (m_CurrEventMsg.busReqMsg.addr);

       if (QueueSize == 0 || m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedStall) {
         m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedNullAck;
       }
       
      if (m_logFileGenEnable){
        if (m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedNullAck) {
          std::cout << "\nSharedMem: " << m_coreId << " has Msg in the RxReq Bus" << std::endl;
          std::cout << "\t\t BusEventName        = " << PrintSharedReqBusEventName(m_CurrEventList.busReqEvent) << std::endl;
          std::cout << "\t\t ReqCoreId           = " << m_CurrEventMsg.busReqMsg.reqCoreId << " RespCoreId = " << m_CurrEventMsg.busReqMsg.wbCoreId 
                    << " Req Addr  = " << m_CurrEventMsg.busReqMsg.addr << " CacheLine = " << reqbusAddrMap.idx_set << std::endl;
          std::cout << "\t\t CacheLine CurrState = " << PrintSharedStateName(m_CurrEventCurrState.busReqEventState) << std::endl;
          std::cout << "\t\t CacheLine NextState = " << PrintSharedStateName(m_CurrEventNextState.busReqEventState) << std::endl;
          std::cout << "\t\t Ctrl ReqAction      = " << PrintSharedActionName(m_CurrEventAction.busReqAction) << " ==================== " << m_cacheCycle << std::endl;
        }
      }
               
      if (m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedNullAck) {
        if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS || 
            m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
          m_Nreqs++;
        }
        
        if (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM) {
          OutStandingExclFlag = RemoveExclRespAddr(); 
        }
        
        if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedFault) {
          std::cout << "SharedCache: DataResp occur in illegal state!" << std::endl;    
          exit(0);     
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedNoAck) {
          // do nothing     
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SharedStall) {
          // do nothing
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendData || 
                 m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendDataExclusive ||
                 m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveResp) {

             
          OutStandingExclFlag = RemoveExclRespAddr();      
          
          sendType = (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendData         ) ? DataOnly    :
                     (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendDataExclusive) ? DataPlsExcl : 
                                                                                                    ExclOnly;
          if (OutStandingExclFlag) { 
            if (sendType == ExclOnly) {
              std::cout << "SharedMem: Fatal Excl processing Error" << std::endl;
              exit(0);
            }
            sendType = DataOnly;
          }
         
          if(m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendExclusiveResp) {
            // save pending write back
            if (!PushMsgInBusTxFIFO (m_CurrEventMsg.busReqMsg.msgId, m_CurrEventMsg.busReqMsg.reqCoreId, m_coreId, m_CurrEventMsg.busReqMsg.addr )) {
              if (m_logFileGenEnable){
                std::cout << "SharedMem: Pending buffer is full !!!!" << std::endl;
              }
              exit(0);
            }
          }
                                    
          if (!DoWriteBack (m_CurrEventMsg.busReqMsg.addr,m_CurrEventMsg.busReqMsg.reqCoreId,m_CurrEventMsg.busReqMsg.msgId, m_cacheCycle*m_dt, sendType)) {
            if (m_logFileGenEnable){
              std::cout << "SharedMem: Cannot Send Data on the bus !!!!" << std::endl;
            }
            exit(0);
          }
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SaveReqCoreId) {
          // save pending write back
          if (!PushMsgInBusTxFIFO (m_CurrEventMsg.busReqMsg.msgId, m_CurrEventMsg.busReqMsg.reqCoreId, m_coreId, m_CurrEventMsg.busReqMsg.addr )) {
            if (m_logFileGenEnable){
              std::cout << "SharedMem: Pending buffer is full !!!!" << std::endl;
            }
            exit(0);
          }
        }
        else if (m_CurrEventAction.busReqAction == SNOOPSharedCtrlAction::SendPendingData) {
          SendPendingReqData(reqbusAddrMap);
        }
        else { // unknown action
          std::cout << "SharedMem: BusReq undefine action occur!" << std::endl;    
          exit(0);     
        }

        // update state into LLC cache 
        if (ProcFromVictimCache) {
          TempCacheline = m_victimCache->ReadCacheLine(VictimCacheLineInfo.line_idx);
          TempCacheline.state = m_CurrEventNextState.busReqEventState;
          m_victimCache->WriteCacheLine(VictimCacheLineInfo.line_idx, TempCacheline);
        }
        else {
          bool UpdateAccessCnt;
          UpdateAccessCnt = (m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetS || 
                             m_CurrEventList.busReqEvent == SNOOPSharedReqBusEvent::GetM);
          UpdateSharedCache (State, m_CurrEventMsg.busReqMsg.addr, m_CurrEventNextState.busReqEventState, NULL, UpdateAccessCnt);
        }
        // update owner core
        if (m_CurrEventOwnerNextState.busReqOwnerState == SNOOPSharedOwnerState::SharedMem) {
          if (ProcFromVictimCache) {
            m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] = m_coreId;
          }
          else {
            m_ownerCoreId[SclWayIdx] = m_coreId;
          }
        }
        else if (m_CurrEventOwnerNextState.busReqOwnerState == SNOOPSharedOwnerState::OtherCore) {
          if (ProcFromVictimCache) {
            m_victimOwnerCoreId[VictimCacheLineInfo.line_idx] = m_CurrEventMsg.busReqMsg.reqCoreId;
          }
          else {
            m_ownerCoreId[SclWayIdx] = m_CurrEventMsg.busReqMsg.reqCoreId;
          }
        }
        m_CurrEventAction.busReqAction = SNOOPSharedCtrlAction::SharedProcessedAck;
      } // if (m_CurrEventAction.busReqAction != SNOOPSharedCtrlAction::SharedNullAck)
        
      bool SkipEviction;
      uint64_t ReplcAddr;
      for (uint32_t i = 0; i < m_victimCacheSize / m_cacheBlkSize ; i++){
        ReplcCacheLine = m_victimCache->ReadCacheLine(i);
        ReplcAddr      = ReplcCacheLine.tag;         
        if (ReplcCacheLine.valid && ReplcCacheLine.evicted == false) {
          QueueSize = m_busIfFIFO->m_rxMsgFIFO.GetQueueSize();
          SkipEviction = false;
          for (int j = 0; j < QueueSize; j++ ) {
            m_CurrEventMsg.busReqMsg  = m_busIfFIFO->m_rxMsgFIFO.GetFrontElement();
            m_busIfFIFO->m_rxMsgFIFO.PopElement();
            m_busIfFIFO->m_rxMsgFIFO.InsertElement(m_CurrEventMsg.busReqMsg);
            if ((m_CurrEventMsg.busReqMsg.addr >> (int) log2(m_cacheBlkSize)) == (ReplcAddr >> (int) log2(m_cacheBlkSize))) {
              SkipEviction = true;
            }
          } 
          if (SkipEviction == false) {
            VictimCacheLineEvict(i);
          }
        }
      }
      
    }
    
    void SharedCacheCtrl::CacheInitialize () {
    
      uint32_t victimCacheLines = m_victimCacheSize / m_cacheBlkSize;
      m_victimCache = CreateObject<VictimCache> (victimCacheLines);
      m_victimCache->SetCacheSize   ( m_victimCacheSize );
      m_victimCache->SetCacheBlkSize( m_cacheBlkSize    );
      m_victimCache->SetCacheLines  ( victimCacheLines  );
    
      if (m_logFileGenEnable){
        std::cout << "\nSharedMem: [Info] Cache State, Owner Initialization" << std::endl; 
      }
       // initialize shared cache states
      InitCacheStates();
      
      // initialize core owner
      for (uint32_t i = 0; i < m_cacheSize/m_cacheBlkSize; i++) {
        m_ownerCoreId[i] = m_coreId;
      }
      
      m_victimOwnerCoreId = new uint16_t[victimCacheLines];
      for (uint32_t i = 0; i < victimCacheLines; i++) {
        m_victimOwnerCoreId[i] = m_coreId;
      }
      
      // load memory content from benchmark files 
      if (m_dramSimEnable == false || m_l2CachePreloadFlag == true) {
        GenericCacheFrmt cacheLine;
        std::stringstream ss;
        std::string fline;
        std::ifstream m_bmTrace;
        size_t pos;
        std::string s;
        uint64_t addr;
        uint32_t setIdx;
        uint32_t nWays;
        uint32_t setOfst;
        bool     filled;
        uint32_t n_miss = 0;
        for (int i = 0; i < m_nPrivCores; i++) {
          ss << m_bmsPath <<"/trace_C" << i << ".trc.shared";
          if (m_logFileGenEnable){
            std::cout << "\nSharedMem: [Info] Load benchmark file\n\t" << ss.str() << std::endl; 
          }
          m_bmTrace.open(ss.str());
          if(m_bmTrace.is_open()) {
            while (getline(m_bmTrace,fline)){
              pos = fline.find(" ");
              s = fline.substr(0, pos); 
              // convert hex string address to decimal 
              addr = (uint64_t) strtol(s.c_str(), NULL, 16);

              GenericCacheMapFrmt addrMap   = m_cache->CpuAddrMap (addr);
              setIdx = addrMap.idx_set;
              nWays  = m_cache -> GetCacheNways();
              setOfst = setIdx * nWays;
              filled = false;
              uint32_t wayIdx;
              //std::cout << "Load Shared Mem addr = " << addr << " setIdx = " << setIdx << " nways = " << nWays <<" setOfst = " <<  setOfst << std::endl;
              for (wayIdx = setOfst; wayIdx < setOfst+nWays;wayIdx++) {
                cacheLine    = m_cache->ReadCacheLine(wayIdx);
                if (cacheLine.valid == true && cacheLine.tag == addrMap.tag) {
                  //std::cout << "SharedMem: [Info] Shared Cache Line exist\n"; 
                  filled = true;
                  //std::cout << "Load Shared Mem addr = " << addr << " tag = " << addrMap.tag << " setIdx = " << setIdx << " nways = " << nWays <<" wayIdx = " <<  wayIdx << std::endl;
                  break;
                }
                else if (cacheLine.valid == false) {
                  //std::cout << "Load Shared Mem addr = " << addr << " tag = " << addrMap.tag << " setIdx = " << setIdx << " nways = " << nWays <<" wayIdx = " <<  wayIdx << std::endl;
                  cacheLine.valid = true;
                  cacheLine.tag   = addrMap.tag;
                  filled = true;
                  m_cache->WriteCacheLine(cacheLine,wayIdx);
                  break;
                }
              }
              if (filled == false) {
                n_miss++;
              }
           }
            m_bmTrace.close();
            ss.str(std::string());
          }
          else {
            if (m_logFileGenEnable){
              std::cout<<"SharedMem: [Error] Benchmark file name "<< m_bmsPath << " is not exist!" << std::endl;
            }
            exit(0); // The program is terminated here
          } 
        } // for (int i = 0; i < m_nPrivCores; i++)
        
        if (n_miss != 0 && m_dramSimEnable == false) {
          std::cout << "SharedMem: [Error] Shared Line cannot placed in cache, increase L2 Cache Size to able to simulate with perfect L2Cache, n_miss = " <<  n_miss << std::endl;
          exit(0);
        }
        else if (n_miss == 0) {
          std::cout << "All data loaded in L2 Cache" << std::endl;
        }
      } // if (m_dramSimEnable == false) {
    }
    
             
    // Cohr FSM Function Call
    void SharedCacheCtrl::CohProtocolFSMProcessing (SNOOPSharedEventType eventType, int state, SNOOPSharedOwnerState owner) {
    
      int state_n;
      SNOOPSharedOwnerState owner_n;
      SNOOPSharedCtrlAction action;
      
      owner_n = owner;
      state_n = state;
      
      IFCohProtocol *ptr;
      
      switch (m_cohrProt) {
        case CohProtType::SNOOP_PMSI: 
          ptr = new PMSI; break;
        case CohProtType::SNOOP_MSI:
          ptr = new MSI; break;
        case CohProtType::SNOOP_MESI:
          ptr = new MESI; break;
        case CohProtType::SNOOP_MOESI:
          ptr = new MOESI; break;
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
        
        ptr->SNOOPSharedEventProcessing (eventType, m_cache2Cache, m_CurrEventList, state_n, owner_n, action);
        
        if (eventType == SNOOPSharedEventType::ReqBus) {
          m_CurrEventCurrState.busReqEventState      = state;
          m_CurrEventNextState.busReqEventState      = state_n;
          m_CurrEventOwnerCurrState.busReqOwnerState = owner;
          m_CurrEventOwnerNextState.busReqOwnerState = owner_n;
          m_CurrEventAction.busReqAction             = action;
        }
        else if (eventType == SNOOPSharedEventType::RespBus) {
          m_CurrEventCurrState.busRespEventState      = state;
          m_CurrEventNextState.busRespEventState      = state_n;
          m_CurrEventOwnerCurrState.busRespOwnerState = owner;
          m_CurrEventOwnerNextState.busRespOwnerState = owner_n;
          m_CurrEventAction.busRespAction             = action;
        }
        else if (eventType == SNOOPSharedEventType::CacheCtrl) {
          m_CurrEventCurrState.CtrlEventState      = state;
          m_CurrEventNextState.CtrlEventState      = state_n;
          m_CurrEventOwnerCurrState.CtrlOwnerState = owner;
          m_CurrEventOwnerNextState.CtrlOwnerState = owner_n;
          m_CurrEventAction.CtrlAction             = action;
        }
        else {
          std::cout << "SharedMem: Error: Known Processing Event" << std::endl;
          exit(0);
        }
        
        delete ptr;
    }
     
    int SharedCacheCtrl::GetWaitDRAMRespState() { 
      switch (m_cohrProt) {
        case CohProtType::SNOOP_MESI:
          return static_cast<int>(SNOOP_MESISharedCacheState::DRAM_d);
        case CohProtType::SNOOP_MSI:
          return static_cast<int>(SNOOP_MSISharedCacheState::DRAM_d);
        case CohProtType::SNOOP_MOESI:
          return static_cast<int>(SNOOP_MOESISharedCacheState::DRAM_d);
        case CohProtType::SNOOP_PMSI: 
          std::cout << "SharedMem: PMSI currently not support DRAM ACQ" << std::endl;
          exit(0);
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
        }
    } 
   
    
    // Initialize Cache Internal States
    void SharedCacheCtrl::InitCacheStates () {
       int initState = ResetCacheState();
        m_cache->InitalizeCacheStates(initState);
        m_victimCache->InitalizeCacheStates(initState);
    }
    
    int SharedCacheCtrl::ResetCacheState () {
      switch (m_cohrProt) {
        case CohProtType::SNOOP_PMSI: 
          return static_cast<int>(SNOOP_PMSISharedCacheState::IorS);
        case CohProtType::SNOOP_MSI:
          return static_cast<int>(SNOOP_MSISharedCacheState::IorS);
        case CohProtType::SNOOP_MESI:
          return static_cast<int>(SNOOP_MESISharedCacheState::I);
        case CohProtType::SNOOP_MOESI:
          return static_cast<int>(SNOOP_MOESISharedCacheState::I);
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
        }
    }
    
    std::string SharedCacheCtrl::PrintSharedStateName (int state) {
      IFCohProtocol *obj;
      std::string sName;
      switch (m_cohrProt) {
        case CohProtType::SNOOP_PMSI: 
          obj = new(PMSI);
          break;
        case CohProtType::SNOOP_MSI:
          obj = new(MSI);
          break;
        case CohProtType::SNOOP_MESI:
          obj = new(MESI);
          break;
        case CohProtType::SNOOP_MOESI:
          obj = new(MOESI);
          break;
        default:
          std::cout << "SharedMem: unknown Snooping Protocol Type" << std::endl;
          exit(0);
      }
      sName = obj->SharedStateName (state);
      delete obj;
      return sName;
    }
       
     std::string SharedCacheCtrl::PrintSharedActionName (SNOOPSharedCtrlAction action)
     {
       switch (action) {
         case SNOOPSharedCtrlAction::SharedNoAck:
           return " No-Action"; 
         case SNOOPSharedCtrlAction::SharedStall:
           return " Stall (Waiting For Data Resp)";
         case SNOOPSharedCtrlAction::SendData:
           return " SendData to core";
         case SNOOPSharedCtrlAction::SendDataExclusive:
           return " Send Exclusive Data to core";
         case SNOOPSharedCtrlAction::SendExclusiveResp:
           return " SendExclusiveResp: Send Exclusive Response to core";
         case SNOOPSharedCtrlAction::StoreData:
           return " StoreData into Memory and Check Pending WB Buffer";
         case SNOOPSharedCtrlAction::StoreDataOnly:
           return " StoreData Only into Memory";
         case SNOOPSharedCtrlAction::SaveReqCoreId:
           return " SaveReqCoreId Msg in Pending WB buffer";
         case SNOOPSharedCtrlAction::SendPendingData:
           return " SendPending Data to core";
         case SNOOPSharedCtrlAction::SendVictimCache:
           return " SendVictimCache";           
         case SNOOPSharedCtrlAction::CopyDRAMIntoCache:
           return " CopyDRAMIntoCache ";          
         case SNOOPSharedCtrlAction::IssueDRAMWrite:
           return " IssueDRAMWrite";
         case SNOOPSharedCtrlAction::IssueCoreInvDRAMWrite:
           return " IssueCoreInvDRAMWrite";
         case SNOOPSharedCtrlAction::IssueCoreInv:
           return " IssueCoreInv";
         case SNOOPSharedCtrlAction::SendExclusiveRespStall:
           return " SendExclusiveRespStall";
         case SNOOPSharedCtrlAction::SharedFault:
           return " Fault" ;
         case SNOOPSharedCtrlAction::SharedNullAck:
           return " No Pending Request";
         case SNOOPSharedCtrlAction::SharedProcessedAck:
           return " Actioned is Processsed";
         default:
           return " Unknown !!!!";
       }
     }

    
     std::string SharedCacheCtrl::PrintSharedRespBusEventName (SNOOPSharedRespBusEvent event)
     {
       switch (event) {
         case SNOOPSharedRespBusEvent::OWnDataResp:
           return " OwnDataRespEvent "; 
         case SNOOPSharedRespBusEvent::OTherDataResp:
           return " OtherDataRespEvent "; 
         case SNOOPSharedRespBusEvent::DRAMDataResp:
           return " DRAMDataResp "; 
         case SNOOPSharedRespBusEvent::NUll:
           return " NullRespEvent ";
         default:
           return "Unknown !!!!";
       }
     }

     std::string SharedCacheCtrl::PrintSharedReqBusEventName (SNOOPSharedReqBusEvent event)
     {
       switch (event) {
         case SNOOPSharedReqBusEvent::GetS:
           return " GetSEvent "; 
         case SNOOPSharedReqBusEvent::GetM:
           return " GetMEvent ";
         case SNOOPSharedReqBusEvent::Upg:
           return " UpgEvent ";
         case SNOOPSharedReqBusEvent::OwnerPutM:
           return " OwnerPutMEvent ";
         case SNOOPSharedReqBusEvent::OTherPutM:
           return " OTherPutMEvent " ;
         case SNOOPSharedReqBusEvent::PutS:
           return " PutSEvent ";
         case SNOOPSharedReqBusEvent::Null:
           return " NullReqEvent ";
         default:
           return "Unknown !!!!";
       }
     }
     
    void SharedCacheCtrl::SetProtocolType (CohProtType ptype) {
      m_cohrProt = ptype;
    }
    
    void SharedCacheCtrl::CycleProcess() {
       CacheCtrlMain();
      // Schedule the next run
      Simulator::Schedule(NanoSeconds(m_dt), &SharedCacheCtrl::Step, Ptr<SharedCacheCtrl > (this));
      m_cacheCycle++;
    }

    // The init function starts the controller at the beginning 
    void SharedCacheCtrl::init() {
        CacheInitialize ();
        Simulator::Schedule(NanoSeconds(m_clkSkew), &SharedCacheCtrl::Step, Ptr<SharedCacheCtrl > (this));
    }

    /**
     * Runs one mobility Step for the given vehicle generator.
     * This function is called each interval dt
     */

    void SharedCacheCtrl::Step(Ptr<SharedCacheCtrl> sharedCacheCtrl) {
        sharedCacheCtrl->CycleProcess();
    }

}
