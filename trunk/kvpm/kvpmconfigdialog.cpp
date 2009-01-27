#include <KConfigSkeleton>
#include <KIconLoader>
#include <KLocale>
#include <KEditListBox>
#include <KTabWidget>
#include <KListWidget>
#include <KColorButton>
#include <KSeparator>

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

  return false;

}

 

KvpmConfigDialog::KvpmConfigDialog( QWidget *parent, QString name, KConfigSkeleton *skeleton ) 
  : KConfigDialog(  parent, name, skeleton), m_skeleton(skeleton) 
{

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
    general->setLayout(general_layout);

    QGridLayout *selection_layout = new QGridLayout();
    general_layout->addLayout(selection_layout);

    KPageWidgetItem  *page_widget_item =  addPage( general, "General"); 
    page_widget_item->setIcon( KIcon("configure") );
}

void KvpmConfigDialog::buildColorsPage()
{
    QWidget *colors = new QWidget;
    QVBoxLayout *colors_layout = new QVBoxLayout();
    colors->setLayout(colors_layout);

    QGroupBox *selection_box = new QGroupBox( i18n("Filesystem types") );
    QGridLayout *selection_layout = new QGridLayout();
    selection_box->setLayout(selection_layout);
    colors_layout->addWidget(selection_box);

    KSeparator *left_separator  = new KSeparator( Qt::Vertical );
    KSeparator *right_separator = new KSeparator( Qt::Vertical );
    left_separator->setLineWidth(2);
    right_separator->setLineWidth(2);
    left_separator->setFrameStyle(  QFrame::Sunken | QFrame::StyledPanel );
    right_separator->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );

    selection_layout->addWidget( left_separator,  0, 2, 5, 1 );
    selection_layout->addWidget( right_separator, 0, 5, 5, 1 );

    m_skeleton->setCurrentGroup("FilesystemColors");
    m_skeleton->addItemColor("ext2",   m_ext2_color);
    m_skeleton->addItemColor("ext3",   m_ext3_color);
    m_skeleton->addItemColor("ext4",   m_ext4_color);
    m_skeleton->addItemColor("reiser",  m_reiser_color);
    m_skeleton->addItemColor("reiser4", m_reiser4_color);
    m_skeleton->addItemColor("msdos", m_msdos_color);
    m_skeleton->addItemColor("jfs",   m_jfs_color);
    m_skeleton->addItemColor("xfs",   m_xfs_color);
    m_skeleton->addItemColor("hfs",   m_hfs_color);
    m_skeleton->addItemColor("none",  m_none_color);
    m_skeleton->addItemColor("free",  m_free_color);
    m_skeleton->addItemColor("swap",  m_swap_color);
    m_skeleton->addItemColor("physvol",  m_physical_color);


    QLabel *ext2_label = new QLabel("ext2");
    selection_layout->addWidget(ext2_label, 0, 0, Qt::AlignRight);
    m_ext2_button = new KColorButton( m_ext2_color );
    selection_layout->addWidget(m_ext2_button, 0, 1, Qt::AlignLeft);

    QLabel *ext3_label = new QLabel("ext3");
    selection_layout->addWidget(ext3_label, 1, 0, Qt::AlignRight);
    m_ext3_button = new KColorButton( m_ext3_color );
    selection_layout->addWidget(m_ext3_button, 1, 1, Qt::AlignLeft);

    QLabel *ext4_label = new QLabel("ext4");
    selection_layout->addWidget(ext4_label, 2, 0, Qt::AlignRight);
    m_ext4_button = new KColorButton( m_ext4_color );
    selection_layout->addWidget(m_ext4_button, 2, 1, Qt::AlignLeft);

    QLabel *reiser_label = new QLabel("reiser");
    selection_layout->addWidget(reiser_label, 0, 3, Qt::AlignRight);
    m_reiser_button = new KColorButton( m_reiser_color );
    selection_layout->addWidget(m_reiser_button, 0, 4, Qt::AlignLeft);

    QLabel *reiser4_label = new QLabel("reiser4");
    selection_layout->addWidget(reiser4_label, 1, 3, Qt::AlignRight);
    m_reiser4_button = new KColorButton( m_reiser4_color );
    selection_layout->addWidget(m_reiser4_button, 1, 4, Qt::AlignLeft);

    QLabel *msdos_label = new QLabel("ms-dos");
    selection_layout->addWidget(msdos_label, 2, 3, Qt::AlignRight);
    m_msdos_button = new KColorButton( m_msdos_color );
    selection_layout->addWidget(m_msdos_button, 2, 4, Qt::AlignLeft);

    QLabel *jfs_label = new QLabel("jfs");
    selection_layout->addWidget(jfs_label, 0, 6, Qt::AlignRight);
    m_jfs_button = new KColorButton( m_jfs_color );
    selection_layout->addWidget(m_jfs_button, 0, 7, Qt::AlignLeft);

    QLabel *xfs_label = new QLabel("xfs");
    selection_layout->addWidget(xfs_label, 1, 6, Qt::AlignRight);
    m_xfs_button = new KColorButton( m_xfs_color );
    selection_layout->addWidget(m_xfs_button, 1, 7, Qt::AlignLeft);

    QLabel *swap_label = new QLabel("linux swap");
    selection_layout->addWidget(swap_label, 2, 6, Qt::AlignRight);
    m_swap_button = new KColorButton( m_swap_color );
    selection_layout->addWidget(m_swap_button, 2, 7, Qt::AlignLeft);

    QLabel *free_label = new QLabel("free space");
    selection_layout->addWidget(free_label, 3, 0, Qt::AlignRight);
    m_free_button = new KColorButton( m_free_color );
    selection_layout->addWidget(m_free_button, 3, 1, Qt::AlignLeft);

    QLabel *none_label = new QLabel("none");
    selection_layout->addWidget(none_label, 3, 3,  Qt::AlignRight);
    m_none_button = new KColorButton( m_none_color );
    selection_layout->addWidget(m_none_button, 3, 4, Qt::AlignLeft);

    QLabel *hfs_label = new QLabel("hfs");
    selection_layout->addWidget(hfs_label, 3, 6,  Qt::AlignRight);
    m_hfs_button = new KColorButton( m_hfs_color );
    selection_layout->addWidget(m_hfs_button, 3, 7, Qt::AlignLeft);

    QLabel *physical_label = new QLabel("physical volumes");
    selection_layout->addWidget(physical_label, 4, 0,  Qt::AlignRight);
    m_physical_button = new KColorButton( m_physical_color );
    selection_layout->addWidget(m_physical_button, 4, 1, Qt::AlignLeft);

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

    //    connect( m_edit_list  , SIGNAL(changed()), this, SLOT(updateConfig()));

} 


void KvpmConfigDialog::updateSettings()
{
    m_search_entries = m_edit_list->items();

    for( int x = 0; x < m_search_entries.size(); x++)
      if( ! m_search_entries[x].endsWith("/") )
	m_search_entries[x].append("/");

    m_ext2_color  = m_ext2_button->color();
    m_ext3_color  = m_ext3_button->color();
    m_ext4_color  = m_ext4_button->color();
    m_xfs_color   = m_xfs_button->color();
    m_jfs_color   = m_jfs_button->color();
    m_hfs_color   = m_hfs_button->color();
    m_swap_color  = m_swap_button->color();
    m_msdos_color = m_msdos_button->color();
    m_none_color  = m_none_button->color();
    m_free_color  = m_free_button->color();
    m_reiser_color   = m_reiser_button->color();
    m_reiser4_color  = m_reiser4_button->color();
    m_physical_color = m_physical_button->color();

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

    m_ext2_button->setColor(Qt::blue);
    m_ext3_button->setColor(Qt::darkBlue);
    m_ext4_button->setColor(Qt::cyan);
    m_swap_button->setColor(Qt::lightGray);
    m_none_button->setColor(Qt::black);
    m_free_button->setColor(Qt::green);
    m_xfs_button->setColor(Qt::darkCyan);
    m_hfs_button->setColor(Qt::darkMagenta);
    m_jfs_button->setColor(Qt::magenta);
    m_msdos_button->setColor(Qt::yellow);
    m_reiser_button->setColor(Qt::red);
    m_reiser4_button->setColor(Qt::darkRed);
    m_physical_button->setColor(Qt::darkGreen);

}

/*
void KvpmConfigDialog::updateConfig()
{
    g_executable_finder->getAllNames();

}
*/

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
