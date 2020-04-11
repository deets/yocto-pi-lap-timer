use std::thread;

use nannou::prelude::*;
use std::sync::mpsc;

use nanomsg::{Socket, Protocol, Error};
use std::io::{Read};
use ringbuf::{RingBuffer, Producer};


mod messageparsing;
use messageparsing::{parse_message, SpiMessage};
mod timetracking;
use timetracking::PropellerTimeTracker;
mod rendering;
use rendering::{Point, ChannelGraph, FPSCounter};


fn connect_to_spi_daemon(addr: &str) -> Result<nanomsg::Socket, Error> {
    match Socket::new(Protocol::Pair)
    {
        Ok(mut socket) => {
            match socket.connect(addr)
            {
                Ok(_) => {
                    return Ok(socket);
                }
                Err(err) => {
                    return Err(err);
                }
            }
        }
        Err(err) => {
            return Err(err);
        }
    }
}


fn main() {
    nannou::app(model)
        .update(update)
        .simple_window(view)
        .run();
}

struct Model {
    message_count: u64,
    data_points_producer: Producer<Point>,
    incoming: std::sync::mpsc::Receiver<SpiMessage>,
    time_tracker: PropellerTimeTracker,
    propeller_time: f32,
    graph: ChannelGraph,
    fps: FPSCounter
}

fn model(app: &App) -> Model {
    let uri = "tcp://fpv-laptimer.local:5000";
    let mut socket = connect_to_spi_daemon(uri).unwrap();
    let rb = RingBuffer::new(20000);
    let (prod, cons) = rb.split();
    let (tx, rx) = mpsc::channel();

    let model = Model {
        message_count: 0,
        data_points_producer: prod,
        incoming: rx,
        time_tracker: PropellerTimeTracker::new(app.time),
        propeller_time: 0.0,
        graph: ChannelGraph::new(cons),
        fps: FPSCounter::new(app.time, 0.05)
    };

    thread::spawn(move || {
        loop {
            let mut buffer = Vec::new();
            match socket.read_to_end(&mut buffer) {
                Ok(_) => {
                    match parse_message(&buffer) {
                        Ok(messages) => {
                            for spi_message in messages {
                                tx.send(spi_message);
                            }
                        }
                        Err(err) => panic!("Problem parsing messages: {:#?}", err)
                    }
                }
                Err(err) => {
                    panic!("Problem reading from socket: {}", err);
                }
            }
        }
    });

    return model;
}

fn update(app: &App, model: &mut Model, _update: Update) {
    for message in model.incoming.try_iter()
    {
        match message {
            SpiMessage::DataMessage(message) => {
                model.propeller_time = model.time_tracker.feed(message.payload[0]);
                model.message_count += 1;
                model.data_points_producer.push(Point{ timestamp: model.propeller_time, value: message.payload[1] });
            }
            SpiMessage::StatisticsMessage(_) => {}
        }
    }
    model.graph.update(app.time);
    model.fps.update(app.time);
}


fn view(app: &App, model: &Model, frame: Frame) {
    let draw = app.draw();
    frame.clear(PURPLE);
    // We'll align to the window dimensions, but padded slightly.
    let win_rect = app.main_window().rect().pad(20.0);
    draw.background().color(PURPLE);

    let text = format!(
        "message count: {}, propeller timestamp: {}, app timestamp: {}, bufsize: {}",
        model.message_count,
        model.propeller_time,
        app.time,
        model.graph.len()
    );

    //draw.text(&text).color(WHITE).font_size(24).wh(win_rect.wh());

    model.graph.draw(&draw, win_rect, app.time);
    model.fps.draw(&draw, win_rect);
    draw.to_frame(app, &frame).unwrap();
}
