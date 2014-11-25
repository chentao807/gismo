// /////////////////////////////////////////////////////////////////
// Generated by axel-plugin wizard
// /////////////////////////////////////////////////////////////////

/* (C) MyCompany */


/* Put a short description of your plugin here */

/* MyCompany-contact@mycompany.com-http://www.mycompany.com */

#pragma once

#include <axlGui/axlInspectorToolFactory.h>

#include "gsAxelPluginExport.h"

class gsGeometryCreatorPrivate;

class GSAXELPLUGIN_EXPORT gsGeometryCreator : public axlInspectorToolInterface {
    Q_OBJECT
    
public:
    gsGeometryCreator(QWidget* parent = 0);
    virtual ~gsGeometryCreator();
    
    static bool registered(void);
    
signals:
    void dataInserted(axlAbstractData* data);
    
public slots:
    void run(void);
    void loadBasis(void);
    void loadGeometry(void);
    void loadMultiPatch(void);

    void update_index(int i);
    void update_degree(int i);

private:
    gsGeometryCreatorPrivate* d;
    
};

dtkAbstractProcess* createProcessCreatorgsGeometryData(void);
axlInspectorToolInterface* creategsGeometryCreator(void);
