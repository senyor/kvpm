
#include <QtGui>

#include "partaddgraphic.h"


PartAddGraphic::PartAddGraphic(QWidget *parent) : QFrame(parent)
{

      setFixedWidth(200);
      setMinimumHeight(30);

      m_preceding_sectors = 0;
      m_following_sectors = 0;
      m_partition_sectors = 1;
}

void PartAddGraphic::paintEvent(QPaintEvent *){

     QPainter painter(this);
     painter.setPen(Qt::blue);

     double offset;
     double length;
     long double total_sectors;

     total_sectors =  m_preceding_sectors + m_following_sectors + m_partition_sectors;

     offset = 0;
     length = (m_preceding_sectors / total_sectors) * 199;
     QRectF preceding_rectangle(offset, 0.0, length, 29.0);

     offset += length;
     length = (m_partition_sectors / total_sectors) * 199;
     if( length < 1.0 )                                     // always show at least a sliver
         length = 1;
     QRectF partition_rectangle(offset, 0.0, length, 29.0);

     offset += length;
     length = (m_following_sectors / total_sectors) * 199;
     QRectF following_rectangle(offset, 0.0, length, 29.0);

     QBrush free_brush( Qt::green, Qt::SolidPattern );
     QBrush partition_brush( Qt::blue, Qt::SolidPattern );

     painter.fillRect( preceding_rectangle, free_brush );
     painter.fillRect( following_rectangle, free_brush );
     painter.fillRect( partition_rectangle, partition_brush );
}

void PartAddGraphic::setPrecedingSectors(long long precedingSectors)
{
  m_preceding_sectors = precedingSectors;
}

void PartAddGraphic::setFollowingSectors(long long followingSectors)
{
  m_following_sectors = followingSectors;

}

void PartAddGraphic::setPartitionSectors(long long partitionSectors)
{
  m_partition_sectors = partitionSectors;

}
