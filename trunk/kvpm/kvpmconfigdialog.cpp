#include <KConfigSkeleton>
#include <KIconLoader>
#include <KLocale>
#include <KEditListBox>
#include <KTabWidget>
#include <KListWidget>

#include <QtGui>
#include <QListWidgetItem>

#include "kvpmconfigdialog.h"



bool config_kvpm()
{

  KConfigSkeleton *skeleton = new KConfigSkeleton();

  KvpmConfigDialog *dialog = new KvpmConfigDialog( NULL, "settings", skeleton );

  dialog->exec();

  return false;

}

 

KvpmConfigDialog::KvpmConfigDialog( QWidget *parent, QString name, KConfigSkeleton *skeleton ) 
  : KConfigDialog(  parent, name, skeleton) 
{

    KIconLoader *icon_loader = KIconLoader::global();

    setFaceType(KPageDialog::List);

    QWidget *kcfg_general = new QWidget;
    QVBoxLayout *general_layout = new QVBoxLayout();
    QLabel *general_label = new QLabel("Not Implemented");
    general_layout->addWidget(general_label);
    kcfg_general->setLayout(general_layout);

    addPage( kcfg_general, 
	     "General", 
	     icon_loader->iconPath("configure", 0), 
	     "General", 
	     true );

    QWidget *kcfg_colors = new QWidget;
    QVBoxLayout *colors_layout = new QVBoxLayout();
    QLabel *colors_label = new QLabel("Not Implemented");
    colors_layout->addWidget(colors_label);
    kcfg_colors->setLayout(colors_layout);


    addPage( kcfg_colors, 
	     "Colors", 
	     icon_loader->iconPath("color-picker", 0), 
	     "Colors", 
	     true );

    buildProgramsPage();

}

KvpmConfigDialog::~KvpmConfigDialog()
{
    delete m_kvpm_config;
    delete m_system_paths_group;
}

void KvpmConfigDialog::buildProgramsPage()
{
    KPageWidgetItem *page_widget_item;

    m_kvpm_config =  new KConfig( "kvpmrc", KConfig::SimpleConfig );
    m_system_paths_group = new KConfigGroup( m_kvpm_config, "SystemPaths" );

    KTabWidget  *kcfg_programs = new KTabWidget;
    QVBoxLayout *programs1_layout = new QVBoxLayout();
    QVBoxLayout *programs2_layout = new QVBoxLayout();
    QWidget *programs1 = new QWidget();
    QWidget *programs2 = new QWidget();
    programs1->setLayout(programs1_layout);
    programs2->setLayout(programs2_layout);

    KEditListBox *edit_list = new KEditListBox();
    programs1_layout->addWidget(edit_list);
    kcfg_programs->insertTab( 1, programs1, "Search path");

    KListWidget *program_list = new KListWidget();
    programs2_layout->addWidget(program_list);
    kcfg_programs->insertTab( 1, programs2, "Executables");

    QStringList search_entries = m_system_paths_group->readEntry( "SearchPath", QStringList() );
    edit_list->insertStringList( search_entries );

    QIcon *check_icon = new KIcon("checkmark");
    QListWidgetItem  *check_item = new QListWidgetItem( *check_icon, "Test"  );
    program_list->addItem( check_item  );

    page_widget_item =  addPage( kcfg_programs, "Programs"); 
    page_widget_item->setIcon( KIcon("applications-system") );

} 


void KvpmConfigDialog::updateSettings()
{
  qDebug() << "Got here....";
}
