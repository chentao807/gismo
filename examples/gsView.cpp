/** @file gsView.cpp

    @brief Produce Paraview file output from XML input, fo Visualizing  G+Smo objects

    This file is part of the G+Smo library.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.

    Author(s): A. Mantzaflaris
*/

#include <iostream>

#include <gismo.h>

using namespace gismo;

int main(int argc, char *argv[])
{
    std::string fn("");
    int numSamples(1000);
    int choice(0);
    bool plot_mesh = false;
    bool plot_net = false;
    bool plot_boundary = false;
    bool get_basis = false;
    bool get_mesh = false;
    bool get_geo = false;
 
    //! [Parse Command line]   
    gsCmdLine cmd("Hi, give me a file (eg: .xml) and I will try to draw it!");  
    
    cmd.addSwitch("geometry", "Try to find and plot a geometry contained in the file", get_geo);
    cmd.addSwitch("mesh"    , "Try to find and plot a mesh contained in the file", get_mesh);
    cmd.addSwitch("basis"   , "Try to find and plot a basis contained in the file", get_basis);
    cmd.addInt   ("s", "samples", "Number of samples to use for viewing", numSamples);
    cmd.addSwitch("element"   , "Plot the element mesh (when applicable)", plot_mesh);
    cmd.addSwitch("controlNet", "Plot the control net (when applicable)", plot_net);
    cmd.addSwitch("boundary"  , "Plot the boundaries and interfaces of patches with colors", plot_boundary);
    cmd.addPlainString("filename", "File containing data to draw (.xml or third-party)", fn);
    
    bool ok = cmd.getValues(argc,argv);
    if ( !ok ) 
    {
        gsWarn << "Something went wrong when reading the command line. Exiting.\n";
        return 1;
    }
    //! [Parse Command line]
    
    if (fn.empty() )
    {
        gsInfo<< cmd.getMessage();
        gsInfo<<"\nType "<< argv[0]<< " -h, to get the list of command line options.\n";
        return 0;
    }

    if (get_basis)
        choice= 3;
    else if (get_mesh)
        choice= 4;
    else if (get_geo)
        choice= 5;
    
    gsFileData<>  filedata(fn);
    
    switch ( choice )
    {
    case 3:
    {
        gsBasis<> * bb = filedata.getAnyFirst< gsBasis<> >();
        if (bb)
            gsInfo<< "Got "<< *bb <<"\n";
        else
        {
            gsInfo<< "Did not find any basis to plot in "<<fn<<", quitting."<<"\n";
            return 0;
        }
        
        gsWriteParaview( *bb , "gsview", numSamples, true);
        
        delete bb;
        break;
    }
    case 4:
    {
        gsMesh<> * msh = filedata.getAnyFirst< gsMesh<> >();
        if (msh)
            gsInfo<< "Got "<< *msh <<"\n";
        else
        {
            gsInfo<< "Did not find any mesh to plot in "<<fn<<", quitting."<<"\n";
            return 0;
        }
        gsWriteParaview( *msh, "gsview");
        delete msh;
        
        break;
    }
    case 5:
    {
        gsGeometry<> * geo = filedata.getAnyFirst< gsGeometry<> >();
        if (geo)
            gsInfo<< "Got "<< *geo <<"\n";
        else
        {
            gsInfo<< "Did not find any geometry to plot in "<<fn<<", quitting."<<"\n";
            return 0;
        }

        gsWriteParaview( *geo , "gsview", numSamples, plot_mesh, plot_net);
        delete geo;
        break;
    }
    default:
        if ( filedata.has< gsMultiPatch<> >() )
        {
            //gsMultiPatch<>* mp = filedata.getFirst< gsMultiPatch<> >();
            gsMultiPatch<> mp;
            filedata.getFirst(mp);
            //gsReadFile<>(fn,mp);// reloads file
            gsInfo<< "Got "<< mp <<"\n";

            if (plot_boundary)
            {
                gsField<>::uPtr nfield = gsFieldCreator<>::boundarySides(mp);
                gsWriteParaview( *nfield, "gsview", numSamples);
            }
            else
            {
                gsWriteParaview(mp, "gsview", numSamples, plot_mesh, plot_net);
            }

            break;
        }
        if ( filedata.has< gsGeometry<> >() )
        {
            std::vector<gsGeometry<>* > geo = filedata.getAll< gsGeometry<> >();
            if ( ! geo.empty() )
                gsInfo<< "Got "<< geo.size() <<" patch"<<(geo.size() == 1 ? "." : "es.") <<"\n";
            else
            {
                gsInfo<< "Problem encountered in file "<<fn<<", quitting." <<"\n";
                return 0;
            }
            
            gsWriteParaview( geo , "gsview", numSamples, plot_mesh, plot_net);
            
            freeAll( geo );

            break;
        }
        
        if ( filedata.has< gsMesh<> >() )
        {
            gsMesh<> * msh = filedata.getFirst< gsMesh<> >();
            if (msh)
                gsInfo<< "Got "<< *msh <<"\n";
            else
            {
                gsInfo<< "Problem encountered in file "<<fn<<", quitting." <<"\n";
                return 0;
            }
            
            gsWriteParaview( *msh, "gsview");
            delete msh;
            break;
        }

        if ( filedata.has< gsBasis<> >() )
        {
            gsBasis<> * bb = filedata.getFirst< gsBasis<> >();
            //bb->uniformRefine(3);
        
            if (bb)
                gsInfo<< "Got "<< *bb <<"\n";
            else
            {
                gsInfo<< "Problem encountered in file "<<fn<<", quitting." <<"\n";
                return 0;
            }
        
            gsWriteParaview( *bb , "gsview", numSamples, plot_mesh);
        
            delete bb;
            break;
        }

        if ( filedata.has< gsSolid<> >() )
        {
            gsSolid<> * bb = filedata.getFirst< gsSolid<> >();

            if (bb)
                gsInfo<< "Got "<< *bb <<"\n";
            else
            {
                gsInfo<< "Problem encountered in file "<<fn<<", quitting." <<"\n";
                return 0;
            }
        
            gsWriteParaviewSolid( *bb, "gsview", numSamples);
            //gsWriteParaview( *bb, "gsview", numSamples, 0, 0.02);
        
            delete bb;
            break;
        }

        if ( filedata.has< gsTrimSurface<> >() )
        {
            gsTrimSurface<> * bb = filedata.getFirst< gsTrimSurface<> >();
            //bb->uniformRefine(3);

            if (bb)
                gsInfo<< "Got "<< *bb <<"\n";
            else
            {
                gsInfo<< "Problem encountered in file "<<fn<<", quitting." <<"\n";
                return 0;
            }

            gsWriteParaview( *bb, "gsview", numSamples);
        
            delete bb;
            break;
        }

        if ( filedata.has< gsPlanarDomain<> >() )
        {
            gsPlanarDomain<> * bb = filedata.getFirst< gsPlanarDomain<> >();
            //bb->uniformRefine(3);

            if (bb)
                gsInfo<< "Got "<< *bb <<"\n";
            else
            {
                gsInfo<< "Problem encountered in file "<<fn<<", quitting." <<"\n";
                return 0;
            }
        
            gsMesh<> * msh = bb->toMesh(numSamples);

            gsWriteParaview( *msh , "gsview");
        
            delete bb;
            delete msh;
            break;
        }
        
        if ( filedata.has< gsMatrix<> >() )
        {
            gsMatrix<> bb;
            filedata.getFirst(bb);
            gsInfo<< "Got Matrix with "<< bb.cols() <<" points.\n";
            gsInfo<< "Plot "<< bb.rows() <<"D points.\n";
            gsWriteParaviewPoints<real_t>( bb, "gsview");
            break;
        }        
        gsInfo<< "Did not find anything to plot in "<<fn<<", quitting."<<"\n";
        return 0;
    }
    
        return system("paraview gsview.pvd &");
}
