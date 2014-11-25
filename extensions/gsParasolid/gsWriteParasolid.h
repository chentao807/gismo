
#pragma once

#include <string>

#include <gsCore/gsForwardDeclarations.h>

typedef int PK_GEOM_t;
typedef int PK_BSURF_t;
struct PK_UVBOX_s;

namespace gismo {

namespace extensions {
    
    /// Writes a gsSurface to a parasolid file
    /// \param gssurf a surface
    /// \param fname filename (without extension)
    template<class T>
    bool gsWriteParasolid(const gsGeometry<T> & ggeo, 
                          std::string const & filename );

    template<class T>
    bool gsWriteParasolid( const gsMultiPatch<T> & gssurfs, std::string const & filename );

    /// Translates a gsTensorBSpline to a PK_BSURF_t
    /// \param[in] bsp B-spline surface
    /// \param[out] bsurf Parasolid spline surface
    template<class T> void
    createPK_BSURF( const gsTensorBSpline<2,T> & bsp, PK_BSURF_t & bsurf);

    /// Translates a gsGeometry to a PK_GEOM_t
    /// \param[in] ggeo inpute G+SMO geometry
    /// \param[out] pgeo Parasolid geometric entity
    template<class T> void
    createPK_GEOM( const gsGeometry<T> & ggeo, PK_GEOM_t & pgeo);

}//extensions

}//gismo
