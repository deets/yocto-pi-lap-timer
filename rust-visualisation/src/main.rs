use std::thread;

use nannou::prelude::*;
use std::sync::mpsc;

use nanomsg::{Socket, Protocol, Error};
use std::io::{Read};
use ringbuf::{RingBuffer, Consumer, Producer};


mod messageparsing;
mod rendering;
mod timetracking;


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
    consumer: Consumer<messageparsing::SpiMessage>,
    producer: Producer<messageparsing::SpiMessage>,
    incoming: std::sync::mpsc::Receiver<messageparsing::SpiMessage>,
    time_tracker: timetracking::PropellerTimeTracker,
    propeller_time: f32
}

fn model(_app: &App) -> Model {
    let uri = "tcp://fpv-laptimer.local:5000";
    let mut socket = connect_to_spi_daemon(uri).unwrap();
    let rb = RingBuffer::new(2000);
    let (mut prod, mut cons) = rb.split();
    let (tx, rx) = mpsc::channel();

    let model = Model {
        message_count: 0,
        consumer: cons,
        producer: prod,
        incoming: rx,
        time_tracker: timetracking::PropellerTimeTracker::new(),
        propeller_time: 0.0
    };

    thread::spawn(move || {
        loop {
            let mut buffer = Vec::new();
            match socket.read_to_end(&mut buffer) {
                Ok(_) => {
                    match messageparsing::parse_message(&buffer) {
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

fn update(_app: &App, model: &mut Model, _update: Update) {
    for message in model.incoming.try_iter()
    {
        model.producer.push(message);
        match message {
            messageparsing::SpiMessage::DataMessage(message) => {
                model.propeller_time = model.time_tracker.feed(message.payload[0]);
                model.message_count += 1;
            }
            messageparsing::SpiMessage::StatisticsMessage(_) => {}
        }
    }
}


fn view(app: &App, model: &Model, frame: Frame) {
    let draw = app.draw();
    frame.clear(PURPLE);
    // We'll align to the window dimensions, but padded slightly.
    let win_rect = app.main_window().rect().pad(20.0);
    draw.background().color(PURPLE);

    let text = format!("message count: {}, propeller timestamp: {}", model.message_count, model.propeller_time);

    draw.text(&text).color(WHITE).font_size(24).wh(win_rect.wh());

    //rendering::draw_graph(&draw, win_rect, );
    draw.to_frame(app, &frame).unwrap();
}
