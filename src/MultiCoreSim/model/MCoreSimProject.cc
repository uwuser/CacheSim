/*
 * File  :      MCoreSimProject.cc
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 15, 2020
 */


#include "MCoreSimProject.h"
#include "ns3/simulator.h"

using namespace std;
using namespace ns3;


/*
 * Create the MCoreSimProject for the supplied configuration data
 */

MCoreSimProject::MCoreSimProject(MCoreSimProjectXml projectXmlCfg) {

  // Set the project xml
  m_projectXmlCfg = projectXmlCfg;

  // Get clock frequency 
  m_dt = projectXmlCfg.GetBusClkInNanoSec();
  m_busCycle = 0;

  // Get Run Till Sim End Flag
  m_runTillSimEnd = projectXmlCfg.GetRunTillSimEnd();

  // Get Simulation time to run
  m_totalTimeInSeconds = (m_runTillSimEnd == true) ? std::numeric_limits<int>::max() : projectXmlCfg.GetTotalTimeInSeconds();

  // Enable Log File Generation
  m_logFileGenEnable = projectXmlCfg.GetLogFileGenEnable();

  // initialize Simulator components
  m_cpuCoreGens  = list<Ptr<CpuCoreGenerator> > ();
  m_cpuFIFO      = list<Ptr<CpuFIFO> > ();
  m_cpuCacheCtrl = list<Ptr<PrivateCacheCtrl> > ();
  m_busIfFIFO    = list<Ptr<BusIfFIFO> > ();

  // Get all cpu configurations from xml
  list<CacheXml> xmlPrivateCaches = projectXmlCfg.GetPrivateCaches();

  CacheXml xmlSharedCache = projectXmlCfg.GetSharedCache();
  
  // Get L1Bus configurations
  L1BusCnfgXml L1BusCnfg = projectXmlCfg.GetL1BusCnfg();
  
  // Get Coherence protocol type
  GetCohrProtocolType();
  
  m_maxPendReq = 0;


  // iterate over each core
  for (list<CacheXml>::iterator it = xmlPrivateCaches.begin(); it != xmlPrivateCaches.end();it++) {
     CacheXml PrivateCacheXml = *it;

     /* 
      * instantiate cpu FIFOs
     */
     Ptr<CpuFIFO> newCpuFIFO = CreateObject<CpuFIFO> ();
     newCpuFIFO->SetFIFOId             (PrivateCacheXml.GetCacheId()  );
     newCpuFIFO->m_txFIFO.SetFifoDepth (projectXmlCfg.GetCpuFIFOSize()); 
     newCpuFIFO->m_rxFIFO.SetFifoDepth (projectXmlCfg.GetCpuFIFOSize()); 
     m_cpuFIFO.push_back               (newCpuFIFO                    );

     /* 
      * instantiate cpu cores
     */
     Ptr<CpuCoreGenerator> newCpuCore = CreateObject<CpuCoreGenerator> (newCpuFIFO);
     stringstream bmTraceFile, cpuTraceFile, ctrlTraceFile;
     bmTraceFile   << projectXmlCfg.GetBMsPath() <<"/trace_C" << PrivateCacheXml.GetCacheId() << ".trc.shared";
     cpuTraceFile  << projectXmlCfg.GetBMsPath() <<"/" << projectXmlCfg.GetCpuTraceFile() << PrivateCacheXml.GetCacheId() << ".txt";
     ctrlTraceFile << projectXmlCfg.GetBMsPath() <<"/" << projectXmlCfg.GetCohCtrlsTraceFile() << PrivateCacheXml.GetCacheId() << ".txt";
     double cpuClkPeriod = PrivateCacheXml.GetCpuClkNanoSec();
     double cpuClkSkew   = cpuClkPeriod * PrivateCacheXml.GetCpuClkSkew() / 100.00;
     newCpuCore->SetCoreId           (PrivateCacheXml.GetCacheId());
     newCpuCore->SetBmFileName       (bmTraceFile.str()           );
     newCpuCore->SetCpuTraceFile     (cpuTraceFile.str()          );
     newCpuCore->SetCtrlsTraceFile   (ctrlTraceFile.str()         );
     newCpuCore->SetDt               (cpuClkPeriod                );
     newCpuCore->SetClkSkew          (cpuClkSkew                  );
     newCpuCore->SetLogFileGenEnable (m_logFileGenEnable          ); 
     m_cpuCoreGens.push_back         (newCpuCore                  );

     /* 
      * instantiate Cache Controller Bus IF FIFOs
     */
     Ptr<BusIfFIFO> newBusIfFIFO = CreateObject<BusIfFIFO> ();
     newBusIfFIFO->SetFIFOId                 (PrivateCacheXml.GetCacheId()  );
     newBusIfFIFO->m_txMsgFIFO.SetFifoDepth  (projectXmlCfg.GetBusFIFOSize()); 
     newBusIfFIFO->m_txRespFIFO.SetFifoDepth (projectXmlCfg.GetBusFIFOSize()); 
     newBusIfFIFO->m_rxMsgFIFO.SetFifoDepth  (projectXmlCfg.GetBusFIFOSize()); 
     newBusIfFIFO->m_rxRespFIFO.SetFifoDepth (projectXmlCfg.GetBusFIFOSize()); 
     m_busIfFIFO.push_back                   (newBusIfFIFO                  );

     /*
      * instantiate cache controllers
      */
     double ctrlClkPeriod = PrivateCacheXml.GetCtrlClkNanoSec();
     double ctrlClkSkew   = ctrlClkPeriod * PrivateCacheXml.GetCtrlClkSkew() / 100.00;
     uint32_t cacheLines  = PrivateCacheXml.GetCacheSize()/PrivateCacheXml.GetBlockSize();
     uint32_t nsets = cacheLines/PrivateCacheXml.GetNWays();
     Ptr<PrivateCacheCtrl> newCacheCtrl = CreateObject<PrivateCacheCtrl> (cacheLines,newBusIfFIFO, newCpuFIFO);
     newCacheCtrl->SetCacheSize           (PrivateCacheXml.GetCacheSize()  );
     newCacheCtrl->SetCacheBlkSize        (PrivateCacheXml.GetBlockSize()  );
     newCacheCtrl->SetCacheNways          (PrivateCacheXml.GetNWays()      );
     newCacheCtrl->SetCacheNsets          (nsets                           );
     newCacheCtrl->SetCacheType           (PrivateCacheXml.GetMappingType());
     newCacheCtrl->SetDt                  (ctrlClkPeriod                   );
     newCacheCtrl->SetClkSkew             (ctrlClkSkew                     );
     newCacheCtrl->SetReqWbRatio          (PrivateCacheXml.GetReqWbRatio() );
     newCacheCtrl->SetCoreId              (PrivateCacheXml.GetCacheId()    );
     newCacheCtrl->SetSharedMemId         (xmlSharedCache.GetCacheId()     );
     newCacheCtrl->SetCache2Cache         (projectXmlCfg.GetCache2Cache()  );
     newCacheCtrl->SetLogFileGenEnable    (m_logFileGenEnable              ); 
     newCacheCtrl->SetMaxPendingReq       (PrivateCacheXml.GetNPendReq()   );
     newCacheCtrl->SetPendingCpuFIFODepth (2*PrivateCacheXml.GetNPendReq() );
     newCacheCtrl->SetProtocolType        (m_cohrProt                      );
     m_cpuCacheCtrl.push_back             (newCacheCtrl                    );

    /* 
     * print configurations
     */
     if (m_logFileGenEnable) {     
       cout << "\nCpuCore       " <<  PrivateCacheXml.GetCacheId() << ", Clk (NanoSec) = " << PrivateCacheXml.GetCpuClkNanoSec()  << ", Clk-Skew (ns) = " << cpuClkSkew  << endl;
       cout << "PrivCacheCtrl " << PrivateCacheXml.GetCacheId()  << ", Clk (NanoSec) = " << PrivateCacheXml.GetCtrlClkNanoSec() << ", Clk-Skew (ns) = " <<  ctrlClkSkew << ", Req2Writeback Ratio = " <<  PrivateCacheXml.GetReqWbRatio() << " %" << endl;
       cout << "\tPrivCache: BlkSize (Bytes) = " << PrivateCacheXml.GetBlockSize() << ", CacheSize (Bytes) = " << PrivateCacheXml.GetCacheSize() << ", MappingType (0:direct, 1: associative) = " << PrivateCacheXml.GetMappingType() << ", Nways = " << PrivateCacheXml.GetNWays() << ", Nsets = " << nsets << endl;
     }
     
     if (m_maxPendReq < PrivateCacheXml.GetNPendReq()) {
       m_maxPendReq = PrivateCacheXml.GetNPendReq();
     }

   }

 /* 
  * instantiate LLC/Shared Memory controller Bus IF FIFOs
  */
   m_sharedCacheBusIfFIFO = CreateObject<BusIfFIFO>  ();
   m_sharedCacheBusIfFIFO->SetFIFOId                 (xmlSharedCache.GetCacheId()   );
   m_sharedCacheBusIfFIFO->m_txMsgFIFO.SetFifoDepth  (projectXmlCfg.GetBusFIFOSize()); 
   m_sharedCacheBusIfFIFO->m_txRespFIFO.SetFifoDepth (projectXmlCfg.GetBusFIFOSize()); 
   m_sharedCacheBusIfFIFO->m_rxMsgFIFO.SetFifoDepth  (projectXmlCfg.GetBusFIFOSize()); 
   m_sharedCacheBusIfFIFO->m_rxRespFIFO.SetFifoDepth (projectXmlCfg.GetBusFIFOSize()); 

 /* 
  * instantiate LLC/Shared Memory -> DRAM controller Bus IF FIFOs
  */  
   m_sharedCacheDRAMBusIfFIFO = CreateObject<DRAMIfFIFO>  ();
   m_sharedCacheDRAMBusIfFIFO->SetFIFOId                 (projectXmlCfg.GetDRAMId()     );
   m_sharedCacheDRAMBusIfFIFO->m_txReqFIFO.SetFifoDepth  (projectXmlCfg.GetBusFIFOSize()); 
   m_sharedCacheDRAMBusIfFIFO->m_rxRespFIFO.SetFifoDepth (projectXmlCfg.GetBusFIFOSize()); 


 /* 
  * instantiate LLC/Shared Memory controller
  */
   double ctrlClkPeriod = xmlSharedCache.GetCtrlClkNanoSec();
   double ctrlClkSkew   = ctrlClkPeriod * xmlSharedCache.GetCtrlClkSkew() / 100.00;
   uint32_t cacheLines  = xmlSharedCache.GetCacheSize()/xmlSharedCache.GetBlockSize();
   uint32_t nsets = cacheLines/xmlSharedCache.GetNWays();
   ReplcPolicy L2ReplcPolicy = ReplcPolicyDecode(xmlSharedCache.GetReplcPolicy());
   m_SharedCacheCtrl = CreateObject<SharedCacheCtrl> (cacheLines, m_sharedCacheBusIfFIFO, m_sharedCacheDRAMBusIfFIFO);
   m_SharedCacheCtrl->SetCacheSize        (xmlSharedCache.GetCacheSize()   );
   m_SharedCacheCtrl->SetCacheBlkSize     (xmlSharedCache.GetBlockSize()   );
   m_SharedCacheCtrl->SetCacheNways       (xmlSharedCache.GetNWays()       );
   m_SharedCacheCtrl->SetCacheNsets       (nsets                           );
   m_SharedCacheCtrl->SetCacheType        (xmlSharedCache.GetMappingType() );
   m_SharedCacheCtrl->SetVictCacheSize    (16*xmlSharedCache.GetBlockSize()); 
   m_SharedCacheCtrl->SetReplcPolicy      (L2ReplcPolicy                   );
   m_SharedCacheCtrl->SetDt               (ctrlClkPeriod                   );
   m_SharedCacheCtrl->SetClkSkew          (ctrlClkSkew                     );
   m_SharedCacheCtrl->SetCoreId           (xmlSharedCache.GetCacheId()     );
   m_SharedCacheCtrl->SetCache2Cache      (projectXmlCfg.GetCache2Cache()  );
   m_SharedCacheCtrl->SetNumPrivCore      (projectXmlCfg.GetNumPrivCore()  );
   m_SharedCacheCtrl->SetBMsPath          (projectXmlCfg.GetBMsPath()      );
   m_SharedCacheCtrl->SetLogFileGenEnable (m_logFileGenEnable              ); 
   m_SharedCacheCtrl->SetProtocolType     (m_cohrProt                      );
   m_SharedCacheCtrl->SetCachePreLoad     (xmlSharedCache.GetCachePreLoad()),
   m_SharedCacheCtrl->SetDramSimEnable    (projectXmlCfg.GetDRAMSimEnable());
   m_SharedCacheCtrl->SetDramFxdLatcy     (projectXmlCfg.GetDRAMFixedLatcy());
   m_SharedCacheCtrl->SetDramModel        (projectXmlCfg.GetDRAMModle()    );
   m_SharedCacheCtrl->SetDramOutstandReq  (projectXmlCfg.GetDRAMOutstandReq());

 /* 
  * instantiate DRAM Controller
  */
   ctrlClkPeriod = projectXmlCfg.GetDRAMCtrlClkNanoSec();
   ctrlClkSkew   = ctrlClkPeriod * projectXmlCfg.GetDRAMCtrlClkSkew() / 100.00;
   m_dramCtrl = CreateObject<DRAMCtrl> (m_sharedCacheDRAMBusIfFIFO);
   m_dramCtrl->SetDramFxdLatcy     (projectXmlCfg.GetDRAMFixedLatcy() );
   m_dramCtrl->SetDramModel        (projectXmlCfg.GetDRAMModle()      );
   m_dramCtrl->SetDramOutstandReq  (projectXmlCfg.GetDRAMOutstandReq()); 
   m_dramCtrl->SetMemCtrlId        (projectXmlCfg.GetDRAMId()         ); 
   m_dramCtrl->SetDt               (ctrlClkPeriod                     );
   m_dramCtrl->SetClkSkew          (ctrlClkSkew                       );   
   m_dramCtrl->SetLogFileGenEnable (m_logFileGenEnable                ); 
 /* 
  * instantiate Interconnect FIFOs
  */
   m_interConnectFIFO = CreateObject<InterConnectFIFO> ();
   m_interConnectFIFO->m_ReqMsgFIFO.SetFifoDepth  (projectXmlCfg.GetBusFIFOSize());
   m_interConnectFIFO->m_RespMsgFIFO.SetFifoDepth (projectXmlCfg.GetBusFIFOSize());


 /* 
  * instantiate bus arbiter
  */
   m_busArbiter = CreateObject<BusArbiter> (m_busIfFIFO, m_sharedCacheBusIfFIFO, m_interConnectFIFO);
   m_busArbiter->SetCacheBlkSize     (xmlSharedCache.GetBlockSize() );
   m_busArbiter->SetDt               (ctrlClkPeriod                 );
   m_busArbiter->SetClkSkew          (ctrlClkSkew                   );
   m_busArbiter->SetNumPrivCore      (projectXmlCfg.GetNumPrivCore());
   m_busArbiter->SetCache2Cache      (projectXmlCfg.GetCache2Cache());
   m_busArbiter->SetNumReqCycles     (L1BusCnfg.GetReqBusLatcy()    );
   m_busArbiter->SetNumRespCycles    (L1BusCnfg.GetRespBusLatcy()   );
   m_busArbiter->SetIsWorkConserv    (L1BusCnfg.GetWrkConservFlag() );
   m_busArbiter->SetBusArchitecture  (L1BusCnfg.GetBusArchitecture());
   m_busArbiter->SetBusArbitration   (L1BusCnfg.GetBusArbitration() );
   m_busArbiter->SetReqBusArb        (L1BusCnfg.GetReqBusArb()      );
   m_busArbiter->SetRespBusArb       (L1BusCnfg.GetRespBusArb()     );
   m_busArbiter->SetCohProtType      (m_cohrProt                    );
   m_busArbiter->SetMaxPendingReq    (m_maxPendReq                  );
   m_busArbiter->SetLogFileGenEnable (m_logFileGenEnable            ); 
   

  /*
   * instantiate LatencyLogger
   */
  std::list<Ptr<CpuFIFO> >::iterator CpuFIFO_itr = m_cpuFIFO.begin();
  std::list<ns3::Ptr<ns3::BusIfFIFO> >::iterator PrivCacheFIFO_itr = m_busIfFIFO.begin();

  for (list<CacheXml>::iterator it = xmlPrivateCaches.begin(); it != xmlPrivateCaches.end();it++) {
     CacheXml PrivateCacheXml = *it;
     Ptr<CpuFIFO> cpuFIFO = (*CpuFIFO_itr);
     Ptr<ns3::BusIfFIFO> privCachBusFIFO = (*PrivCacheFIFO_itr);

     double cpuClkPeriod = PrivateCacheXml.GetCpuClkNanoSec();
     double cpuClkSkew   = cpuClkPeriod * PrivateCacheXml.GetCpuClkSkew() / 100.00;
     stringstream latencyTraceFile, latencyReportFile;
     latencyTraceFile   << projectXmlCfg.GetBMsPath() <<"/LatencyTrace_C" << PrivateCacheXml.GetCacheId() << ".trc";
     latencyReportFile  << projectXmlCfg.GetBMsPath() <<"/LatencyReport_C" << PrivateCacheXml.GetCacheId() << ".csv";

     uint32_t cacheLines  = PrivateCacheXml.GetCacheSize()/PrivateCacheXml.GetBlockSize();
     uint32_t nsets = cacheLines/PrivateCacheXml.GetNWays();

     Ptr<LatencyLogger> newLatencyLogger = CreateObject<LatencyLogger> (m_interConnectFIFO, cpuFIFO, privCachBusFIFO);
     newLatencyLogger->SetDt                (cpuClkPeriod/2.0             );
     newLatencyLogger->SetClkSkew           (cpuClkSkew                   );
     newLatencyLogger->SetCoreId            (PrivateCacheXml.GetCacheId() );
     newLatencyLogger->SetSharedMemId       (xmlSharedCache.GetCacheId()  );
     newLatencyLogger->SetOverSamplingRatio (2                            );
     newLatencyLogger->SetLatencyTraceFile  (latencyTraceFile.str()       );
     newLatencyLogger->SetLatencyReprtFile  (latencyReportFile.str()      );
     newLatencyLogger->SetPrivCacheNsets    ( nsets                       );
     newLatencyLogger->SetPrivCacheBlkSize  (PrivateCacheXml.GetBlockSize());
     newLatencyLogger->SetLogFileGenEnable  ( m_logFileGenEnable           );
     m_latencyLogger.push_back              (newLatencyLogger             );
     CpuFIFO_itr++;
     PrivCacheFIFO_itr++;
  }

    /* 
     * print configurations
     */
     if (m_logFileGenEnable) {     
       cout << "\nSharedMemCtrl " << xmlSharedCache.GetCacheId()  << ", Clk (NanoSec) = " << xmlSharedCache.GetCtrlClkNanoSec() << ", Clk-Skew = (ns) " <<  ctrlClkSkew << endl;
       cout << "	SharedMem: BlkSize (Bytes) = " << xmlSharedCache.GetBlockSize() << ", CacheSize (Bytes) = " << xmlSharedCache.GetCacheSize() << ", MappingType (0:direct, 1: associative) = " << xmlSharedCache.GetMappingType() << ", Nways = " << xmlSharedCache.GetNWays() << ", Nsets = " << nsets << endl;
     }

}

/* 
 * start simulation engines
 */
void MCoreSimProject::Start() {

    for(list<Ptr<CpuCoreGenerator> >::iterator it = m_cpuCoreGens.begin(); it != m_cpuCoreGens.end(); it++) {
        (*it)->init();
    }

    for(list<Ptr<PrivateCacheCtrl> >::iterator it = m_cpuCacheCtrl.begin(); it != m_cpuCacheCtrl.end(); it++) {
        (*it)->init();
    }

    for(list<Ptr<LatencyLogger> >::iterator it = m_latencyLogger.begin(); it != m_latencyLogger.end(); it++) {
        (*it)->init();
    }

    m_SharedCacheCtrl->init();

    m_dramCtrl->init();
    
    m_busArbiter->init();

    Simulator::Schedule(Seconds(0.0), &Step, this);
    Simulator::Stop(MilliSeconds(m_totalTimeInSeconds));
}

void MCoreSimProject::Step(MCoreSimProject* project) {
    project->CycleProcess();
}

void MCoreSimProject::CycleProcess() {
    bool SimulationDoneFlag = true;

    for(list<Ptr<CpuCoreGenerator> >::iterator it = m_cpuCoreGens.begin(); it != m_cpuCoreGens.end(); it++) {
        SimulationDoneFlag &= (*it)->GetCpuSimDoneFlag();
    }

    if (SimulationDoneFlag == true) {
      cout << "Current Simulation Done at Bus Clock Cycle # " << m_busCycle << endl;
      cout << "L2 Nmiss =  " << m_SharedCacheCtrl->GetShareCacheMisses ()   << endl;
      cout << "L2 NReq =  "  << m_SharedCacheCtrl->GetShareCacheNReqs ()    << endl;
      cout << "L2 Miss Rate =  "  << (m_SharedCacheCtrl->GetShareCacheMisses ()/(float)m_SharedCacheCtrl->GetShareCacheNReqs ())*100    << endl;
      exit(0);
    }

    // Schedule the next run
   Simulator::Schedule(NanoSeconds(m_dt), &MCoreSimProject::Step, this);
   m_busCycle++;
  
}

void MCoreSimProject::EnableDebugFlag(bool Enable) {

    for(list<Ptr<PrivateCacheCtrl> >::iterator it = m_cpuCacheCtrl.begin(); it != m_cpuCacheCtrl.end(); it++) {
        (*it)->SetLogFileGenEnable(Enable);
    }

    m_SharedCacheCtrl->SetLogFileGenEnable(Enable);
    m_busArbiter->SetLogFileGenEnable(Enable);
}

void MCoreSimProject::GetCohrProtocolType () {
  string cohType = m_projectXmlCfg.GetCohrProtType();
  if      ( cohType == "MSI"  ) {m_cohrProt = CohProtType::SNOOP_MSI;}
  else if ( cohType == "MESI" ) {m_cohrProt = CohProtType::SNOOP_MESI;}
  else if ( cohType == "MOESI") {m_cohrProt = CohProtType::SNOOP_MOESI;}
  else if ( cohType == "PMSI" ) {m_cohrProt = CohProtType::SNOOP_PMSI;}
  else {std::cout << "Unsupported Coherence Protocol Cnfg Param = " << cohType << std::endl; exit(0);}
}

ReplcPolicy MCoreSimProject::ReplcPolicyDecode (std::string replcPolicy) {
  ReplcPolicy policy;
  if      ( replcPolicy == "RANDOM"  ) {policy = ReplcPolicy::RANDOM;}
  else if ( replcPolicy == "LRU"     ) {policy = ReplcPolicy::LRU;}
  else if ( replcPolicy == "MRU"     ) {policy = ReplcPolicy::MRU;}
  else if ( replcPolicy == "LFU"     ) {policy = ReplcPolicy::LFU;}
  else if ( replcPolicy == "MFU"     ) {policy = ReplcPolicy::MFU;}
  else if ( replcPolicy == "FIFO"    ) {policy = ReplcPolicy::FIFO;}
  else if ( replcPolicy == "LIFO"    ) {policy = ReplcPolicy::LIFO;}
  else {std::cout << "Unsupported Replc Policy Cnfg Param = " << replcPolicy << std::endl; exit(0);}
  return policy;
}



