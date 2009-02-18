// $Id: FragmentStore.cc,v 1.1.2.3 2009/01/30 10:49:57 mommsen Exp $

#include "EventFilter/StorageManager/interface/FragmentStore.h"

using namespace stor;


const bool FragmentStore::addFragment(I2OChain &chain)
{
  // the trivial case that the chain is already complete
  if ( chain.complete() ) return true;

  const FragKey newKey = chain.fragmentKey();

  // Use efficientAddOrUpdates pattern suggested by Item 24 of 
  // 'Effective STL' by Scott Meyers
  fragmentMap::iterator pos = _chains.lower_bound(newKey);

  if(pos != _chains.end() && !(_chains.key_comp()(newKey, pos->first)))
  {
    // key already exists
    pos->second.addToChain(chain);

    if ( pos->second.complete() )
    {
      chain = pos->second;
      _chains.erase(pos);
      return true;
    }
  }
  else
  {
    // The key does not exist in the map, add it to the map
    // Use pos as a hint to insert, so it can avoid another lookup
    _chains.insert(pos, fragmentMap::value_type(newKey, chain));

    // We already handled the trivial case that the chain is complete.
    // Thus, _chains will not have a complete event.
  }

  return false;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -
