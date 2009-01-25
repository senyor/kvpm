#include <KConfigSkeleton>
#include <KIconLoader>
#include <KLocale>
#include <KEditListBox>
#include <KTabWidget>
#include <KListWidget>

#include <QtGui>
#include <QListWidgetItem>
#include <QTableWidget>

#include "kvpmconfigdialog.h"
#include "executablefinder.h"


extern ExecutableFinder *g_executable_finder;


bool config_kvpm()
{

  KConfigSkeleton *skeleton = new KConfigSkeleton();

  KvpmConfigDialog *dialog = new KvpmConfigDialog( NULL, "settings", skeleton );

  dialog->exec();

  return true;

}

 

KvpmConfigDialog::KvpmConfigDialog( QWidget *parent, QString name, KConfigSkeleton *skeleton ) 
  : KConfigDialog(  parent, name, skeleton), m_skeleton(skeleton) 
{

  //    KIconLoader *icon_loader = KIconLoader::global();

    setFaceType(KPageDialog::List);

    buildGeneralPage();
    buildColorsPage();
    buildProgramsPage();

}

KvpmConfigDialog::~KvpmConfigDialog()
{
    delete m_kvpm_config;
    delete m_system_paths_group;
}

void KvpmConfigDialog::buildGeneralPage()
{
    QWidget *general = new QWidget;
    QVBoxLayout *general_layout = new QVBoxLayout();
    QLabel *general_label = new QLabel("Not Implemented Yet");
    general_layout->addWidget(general_label);
    general->setLayout(general_layout);

    KPageWidgetItem  *page_widget_item =  addPage( general, "General"); 
    page_widget_item->setIcon( KIcon("configure") );
}

void KvpmConfigDialog::buildColorsPage()
{
    QWidget *colors = new QWidget;
    QVBoxLayout *colors_layout = new QVBoxLayout();
    QLabel *colors_label = new QLabel("Not Implemented Yet");
    colors_layout->addWidget(colors_label);
    colors->setLayout(colors_layout);


    KPageWidgetItem  *page_widget_item =  addPage( colors, "Colors"); 
    page_widget_item->setIcon( KIcon("color-picker") );
}

void KvpmConfigDialog::buildProgramsPage()
{
    m_executables_table = new QTableWidget();

    KTabWidget  *programs = new KTabWidget;
    QVBoxLayout *programs1_layout = new QVBoxLayout();
    QVBoxLayout *programs2_layout = new QVBoxLayout();
    QWidget *programs1 = new QWidget();
    QWidget *programs2 = new QWidget();
    programs1->setLayout(programs1_layout);
    programs2->setLayout(programs2_layout);

    m_skeleton->setCurrentGroup("SystemPaths");
    m_skeleton->addItemStringList("SearchPath", m_search_entries, QStringList() );

    m_edit_list = new KEditListBox();
    programs1_layout->addWidget(m_edit_list);

    programs->insertTab( 1, programs1, "Search path");
    m_edit_list->insertStringList( m_search_entries );

    programs2_layout->addWidget(m_executables_table);
    programs->insertTab( 1, programs2, "Executables");

    fillExecutablesTable();

    KPageWidgetItem  *page_widget_item =  addPage( programs, "Programs"); 
    page_widget_item->setIcon( KIcon("applications-system") );

    connect( m_edit_list  , SIGNAL(changed()), this, SLOT(updateConfig()));

} 


void KvpmConfigDialog::updateSettings()
{
    m_search_entries = m_edit_list->items();

    for( int x = 0; x < m_search_entries.size(); x++)
      if( ! m_search_entries[x].endsWith("/") )
	m_search_entries[x].append("/");

    m_skeleton->writeConfig();

    g_executable_finder->reload();
    fillExecutablesTable();
}

void KvpmConfigDialog::updateWidgetsDefault()
{
    QStringList default_entries;

    default_entries << "/sbin/" 
		    << "/usr/sbin/" 
		    << "/bin/" 
		    << "/usr/bin/" 
		    << "/usr/local/bin/"
		    << "/usr/local/sbin/";

    m_edit_list->clear();
    m_edit_list->insertStringList( default_entries );
}

void KvpmConfigDialog::updateConfig()
{
    g_executable_finder->getAllNames();

}

void KvpmConfigDialog::fillExecutablesTable()
{

    QTableWidgetItem *table_widget_item = NULL;

    QStringList all_names = g_executable_finder->getAllNames();
    QStringList all_paths = g_executable_finder->getAllPaths();
    QStringList not_found = g_executable_finder->getNotFound();
    int not_found_length = not_found.size();

    m_executables_table->clear();
    m_executables_table->setColumnCount(2);
    m_executables_table->setRowCount( all_names.size() + not_found_length );

    for(int x = 0; x < not_found_length; x++){
        table_widget_item = new QTableWidgetItem( not_found[x] );
	m_executables_table->setItem(x, 0, table_widget_item);

	table_widget_item = new QTableWidgetItem( KIcon("dialog-error"), "Not Found" );
	m_executables_table->setItem(x, 1, table_widget_item);
    }

    // these lists should be the same length, but just in case...

    for(int x = 0; (x < all_names.size()) && (x < all_paths.size()); x++){

        table_widget_item = new QTableWidgetItem( all_names[x] );
	m_executables_table->setItem(x + not_found_length, 0, table_widget_item);

	table_widget_item = new QTableWidgetItem( all_paths[x] );
	m_executables_table->setItem(x + not_found_length, 1, table_widget_item);

    }
    m_executables_table->resizeColumnsToContents();
    m_executables_table->setAlternatingRowColors( true );
    m_executables_table->verticalHeader()->hide();
}


bool KvpmConfigDialog::isDefault()
{
    return false;    // This keeps the "defaults" button enabled
}


bool KvpmConfigDialog::hasChanged()
{
    return true;     // This keeps the "apply" button enabled
}
