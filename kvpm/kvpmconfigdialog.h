#include <KConfigDialog>

#include <QString>


class KConfigSkeleton;
class KEditListBox;
class QTableWidget;

bool config_kvpm();


class KvpmConfigDialog: public KConfigDialog
{
Q_OBJECT

    QTableWidget    *m_executables_table;
    KConfig         *m_kvpm_config;
    KConfigGroup    *m_system_paths_group;
    KEditListBox    *m_edit_list;
    KConfigSkeleton *m_skeleton;
    QStringList      m_search_entries;

    void buildGeneralPage();
    void buildColorsPage();
    void buildProgramsPage();


public:

    KvpmConfigDialog( QWidget *parent, QString name, KConfigSkeleton *skeleton  );
    ~KvpmConfigDialog();

public slots:
    void updateSettings();
    void updateWidgetsDefault();
    void updateConfig();
    void fillExecutablesTable();
    bool isDefault();
    bool hasChanged();
};

