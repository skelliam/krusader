/***************************************************************************
                         kgadvanced.cpp  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kgadvanced.h"
#include "../defaults.h"
#include <klocale.h>
#include <qhbox.h>
#include <qtooltip.h>
#include <sys/param.h>

KgAdvanced::KgAdvanced( bool first, QWidget* parent,  const char* name ) :
      KonfiguratorPage( first, parent, name )
{
  QGridLayout *kgAdvancedLayout = new QGridLayout( parent );
  kgAdvancedLayout->setSpacing( 6 );
  kgAdvancedLayout->setMargin( 11 );

  //  -------------------------- GENERAL GROUPBOX ----------------------------------
  
  QGroupBox *generalGrp = createFrame( i18n( "General" ), parent, "kgAdvGeneralGrp" );
  QGridLayout *generalGrid = createGridLayout( generalGrp->layout() );

  KONFIGURATOR_CHECKBOX_PARAM generalSettings[] =
  //   cfg_class  cfg_name           default     text                                      restart tooltip
    {{"Advanced","AutoMount",        _AutoMount, i18n( "Automount filesystems" ),          false,  i18n( "If checked, Krusader will mount FSTAB mount-points when needed." )}};

  KonfiguratorCheckBoxGroup *generals = createCheckBoxGroup( 2, 0, generalSettings, 1, generalGrp );
  generalGrid->addWidget( generals, 1, 0 );

  addLabel( generalGrid, 2, 0, i18n( "MountMan won't (un)mount the following mount-points:" ),
            generalGrp, "KgAdvLabel2" );
  KonfiguratorEditBox *nonMountPoints = createEditBox( "Advanced", "Nonmount Points", _NonMountPoints, generalGrp, false );
  generalGrid->addWidget( nonMountPoints, 2, 1 );
 
  
#ifdef BSD
  generals->find( "AutoMount" )->setEnabled( false ); /* disable AutoMount on BSD */
#endif
    
  kgAdvancedLayout->addWidget( generalGrp, 0 ,0 );

  //  ----------------------- CONFIRMATIONS GROUPBOX -------------------------------
  
  QGroupBox *confirmGrp = createFrame( i18n( "Confirmations" ), parent, "confirmGrp" );
  QGridLayout *confirmGrid = createGridLayout( confirmGrp->layout() );

  addLabel( confirmGrid, 0, 0, "\n"+i18n( "Krusader will request user confirmation for the following operations:" )+"\n",
            confirmGrp, "KgAdvLabel1" );
            
  KONFIGURATOR_CHECKBOX_PARAM confirmations[] =
  //   cfg_class  cfg_name                default             text                                          restart ToolTip
    {{"Advanced","Confirm Unempty Dir",   _ConfirmUnemptyDir, i18n( "Deleting non-empty directories" ),   false,  ""},
     {"Advanced","Confirm Delete",        _ConfirmDelete,     i18n( "Deleting files" ),                   false,  ""},
     {"Advanced","Confirm Copy",          _ConfirmCopy,       i18n( "Copying files" ),                    false,  ""},
     {"Advanced","Confirm Move",          _ConfirmMove,       i18n( "Moving files" ),                     false,  ""},
     {"Search","Confirm Feed to Listbox",  _ConfirmFeedToListbox, i18n( "Search: Feed to Listbox" ), false, i18n("if checked, Krusader will ask for a result-name; else \"Search result n\" will be used")}};

  KonfiguratorCheckBoxGroup *confWnd = createCheckBoxGroup( 2, 0, confirmations, 5, confirmGrp );

  confirmGrid->addWidget( confWnd, 1, 0 );

  kgAdvancedLayout->addWidget( confirmGrp, 1 ,0 );


  //  ------------------------ FINE-TUNING GROUPBOX --------------------------------

  QGroupBox *fineTuneGrp = createFrame( i18n( "Fine-Tuning" ), parent, "kgFineTuneGrp" );
  QGridLayout *fineTuneGrid = createGridLayout( fineTuneGrp->layout() );
  fineTuneGrid->setAlignment( Qt::AlignLeft | Qt::AlignTop );
  
  QLabel *label = new QLabel( i18n( "Icon cache size (KB):" ), fineTuneGrp, "iconCacheLabel" );
  QToolTip::add( label, i18n( "Cache size determines how fast Krusader can display the contents of a panel; however, too large a cache might consume your memory." ) );
  fineTuneGrid->addWidget( label, 0, 0 );
  KonfiguratorSpinBox *spinBox = createSpinBox( "Advanced", "Icon Cache Size", _IconCacheSize,
                                                1, 8192, fineTuneGrp, false );
  QToolTip::add( spinBox, i18n( "Cache size determines how fast Krusader can display the contents of a panel; however, too large a cache might consume your memory." ) );
  spinBox->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed);
  fineTuneGrid->addWidget( spinBox, 0, 1 );

  addLabel( fineTuneGrid, 1, 0, i18n( "Arguments of updatedb:" ),
            fineTuneGrp, "KgAdvLabel1" );
  KonfiguratorEditBox *updatedbArgs = createEditBox( "Locate", "UpdateDB Arguments", "", fineTuneGrp, false );
  fineTuneGrid->addWidget( updatedbArgs, 1, 1 );
    
  kgAdvancedLayout->addWidget( fineTuneGrp, 2 ,0 );
}

#include "kgadvanced.moc"
