
#include <gsCore/gsConfig.h>

#include <gsCore/gsBoxTopology.h>

namespace gismo
{
  void gsBoxTopology::addAutoBoundaries()
  {
    if ( nboxes == 0 ) {
      return;
    }
    patch_side ps;
    firstPatchSide( ps );
    do {
      if ( !isBoundary( ps ) && !isInterface( ps ) ) {
        addBoundary( ps );
      }
    } while ( nextPatchSide( ps ) );
  }

  bool gsBoxTopology::isInterface( const patch_side& ps ) const
  {
    for ( unsigned i = 0; i < m_interfaces.size(); ++i )
      if ( m_interfaces[i].ps1 == ps || m_interfaces[i].ps2 == ps ) {
        return true;
      }
    return false;
  }

  void gsBoxTopology::checkConsistency() const
  {
    const int numSides = nboxes * 2 * m_dim;      // an n-D cube has 2*d sides
    if ( numSides != 2 * nInterfaces() + nBoundary() ) {
      std::cerr << "*** WARNING *** gsBoxTopology has inconsistent interfaces or boundaries!" <<
                std::endl;
      std::cerr << "                " << size() << " patches with " << numSides << " sides" << std::endl;
      std::cerr << "                " << nInterfaces() << " declared interfaces" << std::endl;
      std::cerr << "                " << nBoundary() << " declared boundaries" << std::endl;
      std::cerr << "                this leaves " << numSides - 2 * nInterfaces() - nBoundary() <<
                " sides unaccounted for" << std::endl;
    }
    for ( const_biterator i = bBegin(); i != bEnd(); ++i )
      if ( i->patch >= nboxes ) {
        std::cerr << "*** WARNING *** gsBoxTopology: box index " << i->patch << " in boundary out of range."
                  << std::endl;
      }
    for ( const_iiterator i = iBegin(); i != iEnd(); ++i )
      if ( i->ps1.patch >= nboxes || i->ps2.patch >= nboxes ) {
        std::cerr << "*** WARNING *** gsBoxTopology: box index " << i->ps1.patch << " or " << i->ps2.patch
                  << " in interface out of range." << std::endl;
      }
  }


  void gsBoxTopology::firstPatchSide( patch_side& result )
  {
    if ( nboxes == 0 ) {
      throw std::runtime_error( "cannot iterate, no patches" );
    }
    boundary::side firstside;
    firstSide( firstside );
    result = patch_side( 0, firstside );
  }

  bool gsBoxTopology::nextPatchSide( patch_side& result )
  {
    if ( nextSide( m_dim, result.side ) ) {
      return true;  // current patch has more sides
    } else {
      result.patch++;                          // go to next patch
      firstSide( result.side );                // go to first side of next patch
      return ( result.patch < ( int )nboxes ); // return true if patch is still valid
    }
  }

  bool gsBoxTopology::getNeighbour(const patch_side& ps ,patch_side& result) const
  {
      for ( unsigned i = 0; i < m_interfaces.size(); ++i ) {
        if ( m_interfaces[i].ps1 == ps ) {
            result = m_interfaces[i].ps2;
            return true;
        }
        else if ( m_interfaces[i].ps2 == ps ) {
            result = m_interfaces[i].ps1;
            return true;
        }
      }
      return false;
  }

  bool gsBoxTopology::getCornerList(const patch_corner& start,std::vector<patch_corner> & cornerList) const
  {
      GISMO_ASSERT(m_dim==2,"works only for 2D");
      cornerList.resize(0);
      std::vector<patch_side> psides;
      getPatchSides(start,m_dim,psides);
      GISMO_ASSERT(psides.size()==2,"there should always be two patchsides on each patch_corner");
      patch_side curSide = psides[0];
      patch_side endSide = psides[1];
      patch_side neighbour;
      patch_corner curCorner = start;
      gsVector<bool> orient;
      gsVector<bool> pars(1);
      bool firstTurn = true;
      do
      {
          cornerList.push_back(curCorner);
          if(!getNeighbour(curSide,neighbour))
          {
              if(firstTurn)
              {
                  patch_side tempSide=curSide;
                  curSide=endSide;
                  endSide=tempSide;
                  curCorner = start;
                  firstTurn = false;
                  if(!getNeighbour(curSide,neighbour))
                      break;
              }
              else
                  break;
          }
          gsVector<bool> parsOnSide;
          getParsOnSide(curCorner,curSide,m_dim,parsOnSide);
          GISMO_ASSERT(parsOnSide.rows()==1,"at the moment this is only for 2D");
          getOrientationOfInterface(curSide,orient);
          pars(0)=orient(0) ? parsOnSide[0] : !parsOnSide[0];
          curCorner = getPatchCorner(neighbour,pars);
          getPatchSides(curCorner,m_dim,psides);
          if(neighbour == psides[0])
              curSide = psides[1];
          else if(neighbour == psides[1])
              curSide = psides[0];
          else
              GISMO_ERROR("one of the two sides has to be the neighbour.");
      }while(!(curCorner==start));
      return firstTurn;
  }

  void gsBoxTopology::getEVs(std::vector<std::vector<patch_corner> > & cornerLists) const
  {
      GISMO_ASSERT(m_dim==2,"works only for 2D");
      cornerLists.clear();
      std::vector<patch_corner> cornerList;
      patch_corner c;
      for(int i = 0;i<nboxes;++i)
      {
          for(int j=1;j<=4;++j)
          {
              c=patch_corner(i,j);
              bool isCycle = getCornerList(c,cornerList);
              bool alreadyReached = false;
              for(unsigned k = 0;k<cornerList.size();++k)
                  if(cornerList[k].patch<i)
                      alreadyReached = true;
              if(isCycle&&cornerList.size()!=4&&!alreadyReached)
                  cornerLists.push_back(cornerList);
          }
      }
  }

  void gsBoxTopology::getOVs(std::vector<std::vector<patch_corner> > & cornerLists) const
  {
      GISMO_ASSERT(m_dim==2,"works only for 2D");
      cornerLists.clear();
      std::vector<patch_corner> cornerList;
      patch_corner c;
      for(int i = 0;i<nboxes;++i)
      {
          for(int j=1;j<=4;++j)
          {
              c=patch_corner(i,j);
              bool isCycle = getCornerList(c,cornerList);
              bool alreadyReached = false;
              for(unsigned k = 0;k<cornerList.size();++k)
                  if(cornerList[k].patch<i)
                      alreadyReached = true;
              if(isCycle&&cornerList.size()==4&&!alreadyReached)
                  cornerLists.push_back(cornerList);
          }
      }
  }
}