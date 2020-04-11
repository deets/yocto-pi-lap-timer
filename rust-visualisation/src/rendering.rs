use nannou::prelude::*;
use nannou::draw::Draw;

use ringbuf::{Consumer};

const TIMERANGE:f32 = 2.0;

pub struct Point {
    pub timestamp: f32,
    pub value: u32
}

pub struct ChannelGraph
{
    points: Consumer<Point>,
    then: f32
}

impl ChannelGraph {

    pub fn new(points: Consumer<Point>) -> ChannelGraph
    {
        ChannelGraph{ points: points, then: 0.0 }
    }

    pub fn update(&mut self, t: f32)
    {
        // cull the points too old
        self.then = t - TIMERANGE;
        let then = self.then;
        self.points.pop_each(
            |point| -> bool {
                point.timestamp < then
            },
            None
        );
    }

    pub fn draw(&self, draw: &Draw, rect: nannou::geom::rect::Rect, now: f32)
    {
        let mut vertices = Vec::new();
        let r = 1.0;
        let g = 1.0;
        let b = 1.0;
        let weight = 8.0;

        self.points.for_each(
            |point| {
                let x = map_range(point.timestamp, self.then, now, rect.left(), rect.right());
                let y = map_range(point.value as f32, 0.0, 1024.0, rect.bottom() * 0.75, rect.top() * 0.75);
                vertices.push((pt2(x, y), srgba(r, g, b, 1.0)));
            }
            );

    draw.polyline()
        .weight(weight)
        .join_round()
        .colored_points(vertices);

    }

    pub fn len(&self) -> usize
    {
        self.points.len()
    }
}

pub struct FPSCounter
{
    last_timestamp: f32,
    fps: f32,
    filter: f32

}

impl FPSCounter {

    pub fn new(now: f32, filter: f32) -> FPSCounter
    {
        FPSCounter{last_timestamp: now, fps: 60.0, filter: filter }
    }

    pub fn  update(&mut self, now: f32)
    {
        let diff = now - self.last_timestamp;
        self.fps = self.fps + (1.0 / diff - self.fps) * self.filter;
        self.last_timestamp = now;
    }

    pub fn draw(&self, draw: &Draw, rect: nannou::geom::rect::Rect)
    {
        let text = format!("FPS: {:.1}", self.fps);
        draw.text(&text).color(WHITE).font_size(18).align_top().xy(rect.bottom_left());
    }
}
