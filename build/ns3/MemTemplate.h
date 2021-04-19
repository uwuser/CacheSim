/*
 * File  :      MemTemplate.h
 * Author:      Salah Hessien
 * Email :      salahga@mcmaster.ca
 *
 * Created On February 16, 2020
 */

#ifndef _MemTemplate_H
#define _MemTemplate_H

#include <queue>

#include <string>

namespace ns3 {
  // Generic FIFO Interface
  template <typename T>
  class GenericFIFO : public ns3::Object {
  private:
     std::queue<T> m_FIFO;
     uint16_t m_fifoDepth;
  public:
    void SetFifoDepth (int fifoDepth) {
      m_fifoDepth = fifoDepth;
    }

    int GetFifoDepth () {
      return m_fifoDepth;
    }

    void InsertElement (T msg) {
      m_FIFO.push(msg);
    }

    void PopElement () {
      m_FIFO.pop();
    }
  
    T GetFrontElement () {
      return m_FIFO.front();
    }

    void UpdateFrontElement (T msg) {
      m_FIFO.front() = msg;
    }

    int GetQueueSize () {
      return m_FIFO.size();
    }

    bool IsEmpty () {
      return m_FIFO.empty();
    }

    bool IsFull () {
      return (m_FIFO.size() == m_fifoDepth) ? true : false;
    }
  };

  // Generic FIFO Interface
  template <typename T>
  class GenericDeque : public ns3::Object {
  private:
     std::deque<T> m_FIFO;
     uint16_t m_fifoDepth;
  public:
    void SetFifoDepth (int fifoDepth) {
      m_fifoDepth = fifoDepth;
    }

    int GetFifoDepth () {
      return m_fifoDepth;
    }

    void InsertElement (T msg) {
      m_FIFO.push_back(msg);
    }

    void InsertElementFront (T msg) {
      m_FIFO.push_front(msg);
    }

    void PopElement () {
      m_FIFO.pop_front();
    }
  
    T GetFrontElement () {
      return m_FIFO.front();
    }

    void UpdateFrontElement (T msg) {
      m_FIFO.front() = msg;
    }

    T GetBackElement () {
      return m_FIFO.back();
    }

    void PopBackElement () {
      m_FIFO.pop_back();
    }

    int GetQueueSize () {
      return m_FIFO.size();
    }

    bool IsEmpty () {
      return m_FIFO.empty();
    }

    bool IsFull () {
      return (m_FIFO.size() == m_fifoDepth) ? true : false;
    }
  };

  // CPU FIFO Interface
  class CpuFIFO : public ns3::Object {
  public:
    enum REQTYPE {
      READ    = 0,
      WRITE   = 1,
      REPLACE = 2
    };
    // A request  contains information on 
    // its own memory request, type, and cycle.
    struct ReqMsg {
      uint64_t msgId;
      uint16_t reqCoreId;
      uint64_t addr;
      uint64_t cycle;
      uint64_t fifoInserionCycle;
      REQTYPE  type;
      uint8_t  data[8];
    };

    struct RespMsg {
      uint64_t msgId;
      uint64_t addr;
      uint64_t reqcycle;
      uint64_t cycle;
    };

    GenericDeque <ReqMsg > m_txFIFO;
    GenericFIFO <RespMsg> m_rxFIFO;

    //GenericDeque <ReqMsg> m_txDeque;

    int m_fifo_id;

    void SetFIFOId (int fifo_id) {
      m_fifo_id = fifo_id;
    }

  };

  // Bus interface FIFO 
  class BusIfFIFO : public ns3::Object {
  public:
    // A request  contains information on 
    // its own memory request, type, and cycle.
    struct BusReqMsg {
      uint64_t addr;
      uint64_t msgId;
      uint16_t reqCoreId;
      uint16_t wbCoreId;
      uint16_t cohrMsgId;
      double   timestamp;
      uint64_t cycle;
      bool     NoGetMResp;
    };

    struct BusRespMsg {
      uint64_t addr;
      uint64_t msgId;
      bool     dualTrans;
      uint16_t reqCoreId;
      uint16_t respCoreId;
      double   timestamp;
      uint64_t cycle;
      uint8_t  data[8];
    };

    GenericDeque<BusReqMsg  > m_txMsgFIFO; // used as a local buffer when it used in SharedMem to save pending requests 
    GenericDeque <BusRespMsg > m_txRespFIFO;
    GenericFIFO <BusReqMsg  > m_rxMsgFIFO;
    GenericFIFO <BusRespMsg > m_rxRespFIFO;

    int m_fifo_id;

    void SetFIFOId (int fifo_id) {
      m_fifo_id = fifo_id;
    }

  };
  
  class DRAMIfFIFO : public ns3::Object {
  public:
    enum DRAM_REQ {
      DRAM_READ    = 0,
      DRAM_WRITE   = 1
    };
    struct DRAMReqMsg {
      uint64_t msgId;
      uint64_t addr;
      DRAM_REQ type;
      uint8_t  data[8];
      uint16_t reqAgentId;
      uint64_t cycle;
      uint64_t dramFetchcycle;
    };
    
    struct DRAMRespMsg {
      uint64_t msgId;
      uint64_t addr;
      uint16_t wbAgentId;
      uint8_t  data[8];
      uint64_t cycle;
    };

    GenericFIFO <DRAMReqMsg  > m_txReqFIFO;
    GenericFIFO <DRAMRespMsg > m_rxRespFIFO;

    int m_fifo_id;

    void SetFIFOId (int fifo_id) {
      m_fifo_id = fifo_id;
    }
  };

  // Bus interface FIFO 
  class InterConnectFIFO : public ns3::Object {
  public:
    GenericFIFO <BusIfFIFO::BusReqMsg  > m_ReqMsgFIFO;
    GenericFIFO <BusIfFIFO::BusRespMsg > m_RespMsgFIFO;
  };

}



#endif /* _MemTemplate */

