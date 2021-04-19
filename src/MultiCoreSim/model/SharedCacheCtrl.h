/*
 * File  :      SharedCacheCtrl.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 20, 2020
 */

#ifndef _SharedCacheCtrl_H
#define _SharedCacheCtrl_H

#include "ns3/ptr.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "MemTemplate.h"
#include "GenericCache.h"
#include "PMSI.h"
#include "MSI.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "CohProtocolCommon.h"

namespace ns3 { 
  /**
   * brief SharedCacheCtrl Implements Cache coherence protocol on the 
   * shared memory side, it is the main interface between LLC and the 
   * bus arbiter, the cache controller communicates with bus arbiter 
   * through FIFO bi-directional channel. 
  */
  
  class SharedCacheCtrl : public ns3::Object {
  private:
     enum CacheField {
       State = 0,
       Tag,
       Data,
       Line
     };
     
     enum SendDataType {
       DataOnly = 0,
       ExclOnly,
       DataPlsExcl,
       CoreInv
     };
     
     // shared cache parameters
     uint16_t   m_cacheType;
     uint32_t   m_cacheSize;
     uint32_t   m_cacheBlkSize;
     uint32_t   m_victimCacheSize;
     uint32_t   m_nways;
     uint32_t   m_nsets;
     int        m_coreId;
     double     m_dt; 
     double     m_clkSkew; 
     uint64_t   m_cacheCycle;
     bool       m_cache2Cache;
     int        m_nPrivCores;
     bool       m_logFileGenEnable;
     bool       m_l2CachePreloadFlag;
     bool       m_dramSimEnable;
     uint32_t   m_dramLatcy;
     uint32_t   m_dramOutstandReq;
     
     uint64_t   m_Nreqs;
     uint64_t   m_Nmiss;
     
     std::string m_dramModle;
     
     ReplcPolicy m_replcPolicy;
     
     SNOOPSharedMsgList        m_CurrEventMsg;
     SNOOPSharedEventList      m_CurrEventList;
     SNOOPSharedActionList     m_CurrEventAction;  
     SNOOPSharedStateList      m_CurrEventCurrState,
                               m_CurrEventNextState;
     SNOOPSharedOwnerList      m_CurrEventOwnerCurrState,
                               m_CurrEventOwnerNextState;
     
     CohProtType m_cohrProt;

     std::string m_bmsPath;

     uint16_t * m_ownerCoreId;
     
     uint16_t * m_victimOwnerCoreId;

     Ptr<BusIfFIFO> m_busIfFIFO;
     
     Ptr<DRAMIfFIFO> m_dramBusIfFIFO;
     
     GenericFIFO <BusIfFIFO::BusReqMsg> m_PndWBFIFO;
     
     GenericFIFO <uint64_t> m_DRAMOutStandingExclMsg;

     Ptr<GenericCache> m_cache;
     
     Ptr<VictimCache> m_victimCache;

     SNOOPSharedReqBusEvent  ChkBusRxReqEvent  (BusIfFIFO::BusReqMsg &  busReqMsg); 

     SNOOPSharedRespBusEvent ChkBusRxRespEvent (BusIfFIFO::BusRespMsg & busRespMsg); 
     
     SNOOPSharedCtrlEvent ChkDRAMReqEvent (SNOOPSharedReqBusEvent busReqEvent);

     void UpdateSharedCache                     (CacheField field, 
                                                 uint64_t addr, 
                                                 int state, 
                                                 uint8_t * data,
                                                 bool UpdateAccessCnt);
                                                 
     void VictimCacheLineEvict (uint32_t victimWayIdx);
     
     void SendExclRespEarly ();
     
     bool RemoveExclRespAddr ();

     bool FetchLine (uint64_t addr, GenericCacheFrmt & cacheLine, uint32_t & LineWayIdx);

     bool DoWriteBack (uint64_t cl_idx, uint16_t wbCoreId, uint64_t msgId, double timestamp , SendDataType type);

     bool PushMsgInBusTxFIFO  (uint64_t       msgId, 
                                                uint16_t       reqCoreId, 
                                                uint16_t       wbCoreId, 
                                                uint64_t       addr);

     void SendPendingReqData  (GenericCacheMapFrmt recvTrans );

     void CycleProcess  ();
     
     void CacheCtrlMain ();
     
     void CacheInitialize();
     
    void CohProtocolFSMProcessing (SNOOPSharedEventType eventType, int state, SNOOPSharedOwnerState owner);
    
    int GetWaitDRAMRespState();
    
    void InitCacheStates ();
    
    int ResetCacheState ();
        
    bool SendDRAMReq (uint64_t msgId, uint64_t addr, DRAMIfFIFO::DRAM_REQ type);
    
    std::string PrintSharedActionName (SNOOPSharedCtrlAction action);
    
    std::string PrintSharedRespBusEventName (SNOOPSharedRespBusEvent event);
    
    std::string PrintSharedReqBusEventName (SNOOPSharedReqBusEvent event);
    
    std::string PrintSharedStateName (int state);

  public:
    static TypeId GetTypeId(void);

    SharedCacheCtrl (uint32_t       cachLines, 
                     Ptr<BusIfFIFO> assoicateBusIfFIFO,
                     Ptr<DRAMIfFIFO> associatedDRAMBusIfFifo);


    ~SharedCacheCtrl();
     
    void SetCacheSize (uint32_t cacheSize);

    uint32_t GetCacheSize ();

    void SetCacheBlkSize (uint32_t cacheBlkSize);

    uint32_t GetCacheBlkSize ();

    void SetCacheNways (uint32_t nways);

    uint32_t GetCacheNways ();

    void SetCacheNsets (uint32_t nsets);

    uint32_t GetCacheNsets ();

    void SetCacheType (uint16_t cacheType);
    
    void SetVictCacheSize (uint32_t cacheSize);

    uint16_t GetCacheType ();

    void SetCoreId (int coreId);

    int GetCoreId ();

    void SetDt (double dt);

    int GetDt ();

    void SetClkSkew (double clkSkew);

    void SetCache2Cache (bool cache2Cache);
    
    void SetReplcPolicy (ReplcPolicy replcPolicy);

    void SetBMsPath  (std::string bmsPath);

    void SetNumPrivCore (int nPrivCores);
    
    void SetProtocolType (CohProtType ptype);

    void SetLogFileGenEnable (bool logFileGenEnable);
    
    void SetCachePreLoad (bool l2CachePreload);
    
    void SetDramSimEnable (bool dramSim_en );
    
    void SetDramFxdLatcy (uint32_t dramLatcy );
    
    void SetDramModel (std::string dramModel );
    
    void SetDramOutstandReq (uint32_t dramOutstandReqs);
    
    uint64_t GetShareCacheMisses ();
    
    uint64_t GetShareCacheNReqs ();
    
    void init();

     static void Step(Ptr<SharedCacheCtrl> sharedCacheCtrl);

  };
}

#endif /* _SharedCacheCtrl_H */

