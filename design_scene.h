#ifndef LED_DESIGN_SCENE_H
#define LED_DESIGN_SCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsEllipseItem>
#include <QGraphicsView>
#include <QGraphicsTextItem>
#include <QDebug>
#include <math.h>
#include <QList>
#include <QColor>
#include <QFile>
class design_scene;
class pattern
{
public:
    pattern(qint8 _total_time, qint8 _mid, qint8 _offset, QColor _start_color, QColor _end_color):
        total_time(_total_time),
        mid(_mid),
        offset(_offset),
        start_color(_start_color),
        end_color(_end_color) {}
    pattern() {}
    qint8 total_time;
    qint8 mid;
    qint8 offset;
    QColor start_color;
    QColor end_color;
    bool is_solid;

    inline QString toString()
    {
        QString ret;
        if(is_solid) {
            QTextStream(&ret) << "Solid:\n" << start_color.name() << "," <<
                          total_time << "," << offset;
        } else {
            QTextStream(&ret) << "Pattern:\n" << start_color.name() << "," << end_color.name() << "," <<
                          total_time << "," << offset << "," << mid;
        }
        return ret;
    }
};


class led_strip : public QObject
{
    Q_OBJECT
public:
    led_strip(design_scene *s);
    qint32 add_led(QGraphicsEllipseItem* led, QPointF loc);
    struct led_instance{
        QGraphicsEllipseItem *led;
        QGraphicsSimpleTextItem *id;
        QPointF loc;
        QList<pattern> pattern_list;
        led_instance(QGraphicsEllipseItem *_led, QPointF _loc, QGraphicsSimpleTextItem *_id) :
            led(_led),
            loc(_loc),
            id(_id) {}
    };
    qint8 led_at_pos(QPointF pt);
    void set_led_pos(uint8_t led_id, QPointF loc);
    QGraphicsEllipseItem* get_led_byid(qint8 led_id);
    void add_pattern(qint8 led_id, pattern patt);
    void save_to_file(QString& file_name);
    void set_loop_time(quint8 loop_time) { global_loop_time = loop_time; }
    inline QList<pattern> get_led_pattern_list(qint8 led_id)
    {
        return strip[led_id].pattern_list;
    }
public slots:
    void loop_player();
private:
    quint8 global_loop_time;
    quint16 cnt;
    qint8 num_leds;
    design_scene *scene;
    QList<led_instance> strip;
    QColor get_color_at_time(qint8 led_id,qint16 time);
};

class design_scene : public QGraphicsScene
{
    Q_OBJECT
public:
    design_scene(QObject * parent = 0);
    void set_led_color(qint8 led_id, QColor color);
    void push_led_pattern(qint8 selected_led_id ,pattern curr_pattern);
    const led_strip* get_led_strip() { return strip; }
    void set_loop_time(quint8 loop_time) { strip->set_loop_time(loop_time); }
    inline QList<pattern> get_pattern_list(qint8 led_id)
    {
        return strip->get_led_pattern_list(led_id);
    }
    void save_patterns_to_file(QString& file_name);

signals:

private:
    led_strip* strip;
    QPointF get_snap_coords(QPointF pt);
    qint32 coord_step;
    bool repos_event;
    qint8 repos_led_id;
signals:
    void led_selected(qint8 led_id);
public slots:
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);
};


#endif // LED_DESIGN_SCENE_H
