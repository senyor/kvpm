#include <KConfigDialog>

#include <QString>


class KConfigSkeleton;



bool config_kvpm();


class KvpmConfigDialog: public KConfigDialog
{
Q_OBJECT


    KConfig      *m_kvpm_config;
    KConfigGroup *m_system_paths_group;

    void buildProgramsPage();

public:

    KvpmConfigDialog( QWidget *parent, QString name, KConfigSkeleton *skeleton  );
    ~KvpmConfigDialog();

private slots:
    void updateSettings();

};

