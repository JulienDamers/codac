/** 
 *  Hashcode classes
 * ----------------------------------------------------------------------------
 *  \date       2020
 *  \author     Simon Rohou
 *  \copyright  Copyright 2020 Simon Rohou
 *  \license    This program is distributed under the terms of
 *              the GNU Lesser General Public License (LGPL).
 */

#include "tubex_Hashcode.h"
#include "tubex_CtcEval.h"
#include "tubex_CtcDeriv.h"
#include "tubex_CtcDist.h"

using namespace std;
using namespace ibex;

namespace tubex
{
  // ContractorHashcode class
  
  ContractorHashcode::ContractorHashcode(const Contractor& ctc)
  {
    m_n = ctc.m_v_domains.size()+1;
    m_ptr = new std::uintptr_t[m_n];

    for(size_t i = 0 ; i < m_n-1 ; i++)
      m_ptr[i] = DomainHashcode::uintptr(*ctc.m_v_domains[i]);

    switch(ctc.m_type)
    {
      case Contractor::Type::T_EQUALITY:
        m_ptr[m_n-1] = 0; // todo: check this
        break;

      case Contractor::Type::T_COMPONENT:
        m_ptr[m_n-1] = 1; // todo: check this
        break;
        
      case Contractor::Type::T_IBEX:
        m_ptr[m_n-1] = reinterpret_cast<std::uintptr_t>(&ctc.m_static_ctc.get());
        assert(m_ptr[m_n-1] > 4); // reserved codes
        break;

      case Contractor::Type::T_TUBEX:

        if(typeid(ctc.m_dyn_ctc.get()) == typeid(CtcEval))
          m_ptr[m_n-1] = 2;

        else if(typeid(ctc.m_dyn_ctc.get()) == typeid(CtcDeriv))
          m_ptr[m_n-1] = 3;

        else if(typeid(ctc.m_dyn_ctc.get()) == typeid(CtcDist))
          m_ptr[m_n-1] = 4;

        else
        {
          m_ptr[m_n-1] = reinterpret_cast<std::uintptr_t>(&ctc.m_dyn_ctc.get());
          assert(m_ptr[m_n-1] > 4); // reserved codes
        }

        break;

      default:
        assert(false && "unhandled case");
    }
  }

  bool ContractorHashcode::operator<(const ContractorHashcode& a) const
  {
    for(size_t i = 0 ; i < min(m_n, a.m_n) ; i++)
    {
      if(m_ptr[i] == a.m_ptr[i])
        continue;

      return m_ptr[i] < a.m_ptr[i];
    }

    return false;
  }

  // DomainHashcode class

  DomainHashcode::DomainHashcode(const Domain& dom)
  {
    m_ptr = DomainHashcode::uintptr(dom);
  }

  bool DomainHashcode::operator<(const DomainHashcode& a) const
  {
    return m_ptr < a.m_ptr;
  }

  uintptr_t DomainHashcode::uintptr(const Domain& dom)
  {
    uintptr_t ptr = 0;

    switch(dom.m_memory_type)
    {
      case Domain::MemoryRef::M_DOUBLE:
        ptr = reinterpret_cast<std::uintptr_t>(&dom.m_ref_memory_d.get());
        break;

      case Domain::MemoryRef::M_INTERVAL:
        ptr = reinterpret_cast<std::uintptr_t>(&dom.m_ref_memory_i.get());
        break;

      case Domain::MemoryRef::M_VECTOR:
        ptr = reinterpret_cast<std::uintptr_t>(&dom.m_ref_memory_v.get());
        break;

      case Domain::MemoryRef::M_INTERVAL_VECTOR:
        ptr = reinterpret_cast<std::uintptr_t>(&dom.m_ref_memory_iv.get());
        break;

      case Domain::MemoryRef::M_SLICE:
        ptr = reinterpret_cast<std::uintptr_t>(&dom.m_ref_memory_s.get());
        break;

      case Domain::MemoryRef::M_TUBE:
        ptr = reinterpret_cast<std::uintptr_t>(&dom.m_ref_memory_t.get());
        break;

      case Domain::MemoryRef::M_TUBE_VECTOR:
        ptr = reinterpret_cast<std::uintptr_t>(&dom.m_ref_memory_tv.get());
        break;

      default:
        assert(false && "unhandled case");
    }

    return ptr;
  }
}