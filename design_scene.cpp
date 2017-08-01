#include "design_scene.h"

design_scene::design_scene(QObject * parent) :
    QGraphicsScene(parent),
    coord_step(10),
    repos_event(false)
{
    strip = new led_strip(this);
}

void design_scene::set_led_color(qint8 led_id, QColor color)
{
    if(strip->get_led_byid(led_id) != nullptr) {
        strip->get_led_byid(led_id)->setBrush(QBrush(color, Qt::SolidPattern));
    }
}

void design_scene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    QPointF pt = get_snap_coords(QPointF(mouseEvent->scenePos().x(), mouseEvent->scenePos().y()));
    qint8 led_id = strip->led_at_pos(pt);
    if(led_id == -1) {
        qint32 led_id = strip->add_led(this->addEllipse(pt.x(), pt.y(), coord_step*2.0, coord_step*2.0,
                        QPen(), QBrush(Qt::white,Qt::SolidPattern)), pt);

    } else {
        emit led_selected(led_id);
    }
}
void design_scene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    QPointF pt = get_snap_coords(QPointF(mouseEvent->scenePos().x(), mouseEvent->scenePos().y()));
    qint8 led_id = strip->led_at_pos(pt);
    if(led_id >= 0) {
        for(uint i = 0 ; i < views().length(); i++) {
            views()[i]->viewport()->setCursor(Qt::ClosedHandCursor);
        }
    }
    repos_event = true;
    repos_led_id = led_id;
}

void design_scene::mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    for(uint i = 0 ; i < views().length(); i++) {
        views()[i]->viewport()->unsetCursor();
    }
    repos_event = false;
}

void design_scene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent)
{
    if(repos_event) {
        QPointF pt = get_snap_coords(QPointF(mouseEvent->scenePos().x(), mouseEvent->scenePos().y()));
        strip->set_led_pos(repos_led_id, pt);
        update(sceneRect());
        for(uint i = 0 ; i < views().length(); i++) {
            views()[i]->viewport()->setCursor(Qt::ClosedHandCursor);
        }
        qDebug()<<"Pos " << pt;
    }
}

void design_scene::push_led_pattern(qint8 selected_led_id ,pattern curr_pattern)
{
    strip->add_pattern(selected_led_id, curr_pattern);
}

QPointF design_scene::get_snap_coords(QPointF pt)
{
    return QPointF(roundf((pt.x() - coord_step)/(coord_step*2))*(coord_step*2),
                   roundf((pt.y() - coord_step)/(coord_step*2))*(coord_step*2));
}

led_strip::led_strip(design_scene *s) :
    scene(s)
{
    num_leds = 0;
}

qint32 led_strip::add_led(QGraphicsEllipseItem *led, QPointF loc)
{
    led->setFlag(QGraphicsEllipseItem::ItemIsMovable, true);
    strip.append(led_instance(led,loc));
    num_leds = strip.length();
    QGraphicsSimpleTextItem* id_name = scene->addSimpleText(QString::number(num_leds - 1));
    id_name->setBrush(Qt::black);
    id_name->setParentItem(led);
    id_name->setPos(loc.x() + 2.5, loc.y() + 2.5);
    qDebug() << "Created LED ID" << num_leds - 1 << " @ " << loc.x() << " " << loc.y();
    return num_leds;
}

qint8 led_strip::led_at_pos(QPointF loc)
{
    for(int i = 0; i < strip.length(); i++) {
        if(strip[i].loc == loc) {
            return i;
        }
    }
    return -1;
}

QGraphicsEllipseItem* led_strip::get_led_byid(qint8 led_id)
{
    return strip[led_id].led;
}

void led_strip::set_led_pos(uint8_t led_id, QPointF loc)
{
    if(strip.length() <= 0) {
        return;
    }
    for(int i = 0; i < strip.length(); i++) {
        if(strip[i].loc == loc) {
            return;
        }
    }
    qDebug() << "REPOS " << loc;
    QPointF offset =  loc - strip[led_id].loc;
    strip[led_id].led->moveBy(offset.x(), offset.y());
    strip[led_id].loc = loc;
}

void led_strip::add_pattern(qint8 led_id, pattern patt)
{
    strip[led_id].pattern_list.append(patt);
}

QColor led_strip::get_color_at_time(qint8 led_id, qint16 time)
{
    quint16 completed_time = 0;
    quint8 step = 0;
    QColor ret = Qt::black;
    for(quint16 i = 0; i < strip[led_id].pattern_list.length(); i++)
    {
        completed_time += strip[led_id].pattern_list[i].offset;
        if(time < completed_time) {
            return Qt::black;
        }
        completed_time += strip[led_id].pattern_list[i].total_time;
        if(time < completed_time) {
            if(strip[led_id].pattern_list[i].is_solid) {
                return strip[led_id].pattern_list[i].start_color;
            } else {
                float t_frac = float(completed_time - time)/strip[led_id].pattern_list[i].total_time;
                QColor s_color = strip[led_id].pattern_list[i].start_color;
                QColor e_color = strip[led_id].pattern_list[i].end_color;
                float mid = strip[led_id].pattern_list[i].mid/100.0;
                if(t_frac <= mid) {
                    t_frac /= mid;
                } else {
                    t_frac = (1.0 - t_frac)/(1.0 - mid);
                }
                return QColor(t_frac*e_color.red() + (1.0-t_frac) * s_color.red(),
                              t_frac*e_color.green() + (1.0-t_frac) * s_color.green(),
                              t_frac*e_color.blue() + (1.0-t_frac) * s_color.blue());
            }
        }
    }
    return ret;
}

void led_strip::loop_player()
{
    QColor curr_color;
    for(qint8 i = 0; i < strip.length(); i++)
    {
        curr_color = get_color_at_time(i, cnt);
        if(strip[i].led->brush().color() != curr_color)
        {
            qDebug() << cnt << " " << curr_color.name();
            strip[i].led->setBrush(QBrush(curr_color));
        }
    }
    cnt++;
    if(cnt > global_loop_time*100 - 1) {
        cnt = 0;
    }
}
