#include <KConfigDialog>
#include <KConfigSkeleton>
#include <KColorButton>
#include <KEditListBox>

#include <QTableWidget>
#include <QStringList>
#include <QColor>
#include <QCheckBox>

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

    KColorButton *m_ext2_button,
                 *m_ext3_button,
                 *m_ext4_button,
                 *m_reiser_button,
                 *m_reiser4_button,
                 *m_msdos_button,
                 *m_swap_button,
                 *m_xfs_button,
                 *m_jfs_button,
                 *m_hfs_button,
                 *m_free_button,
                 *m_none_button,
                 *m_physical_button;

    QColor m_ext2_color,
           m_ext3_color,
           m_ext4_color,
           m_reiser_color,
           m_reiser4_color,
           m_msdos_color,
           m_swap_color,
           m_xfs_color,
           m_jfs_color,
           m_hfs_color,
           m_free_color,
           m_none_color,
           m_physical_color;

    QCheckBox *m_device_check,      *m_volume_check,
              *m_partition_check,   *m_size_check,
              *m_capacity_check,    *m_type_check,
              *m_used_check,        *m_filesystem_check,
              *m_usage_check,       *m_stripes_check,
              *m_group_check,       *m_stripesize_check,
              *m_flags_check,       *m_snapmove_check,
              *m_mount_check,       *m_state_check,
                                    *m_access_check;


    bool m_device_column,      m_volume_column,
         m_partition_column,   m_size_column,
         m_capacity_column,    m_type_column,
         m_used_column,        m_filesystem_column,
         m_usage_column,       m_stripes_column,
         m_group_column,       m_stripesize_column,
         m_flags_column,       m_snapmove_column,
         m_mount_column,       m_state_column,
                               m_access_column;

    void buildGeneralPage();
    void buildColorsPage();
    void buildProgramsPage();


public:

    KvpmConfigDialog( QWidget *parent, QString name, KConfigSkeleton *skeleton  );
    ~KvpmConfigDialog();

public slots:
    void updateSettings();
    void updateWidgetsDefault();
    //    void updateConfig();
    void fillExecutablesTable();
    bool isDefault();
    bool hasChanged();
};

