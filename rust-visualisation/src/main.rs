use std::thread;

use nannou::prelude::*;

use nanomsg::{Socket, Protocol, Error};
use std::io::{Read};
use ringbuf::{RingBuffer, Consumer};


mod messageparsing;

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
    incoming: Consumer<messageparsing::SpiMessage>,
}

fn model(_app: &App) -> Model {
    let uri = "tcp://fpv-laptimer.local:5000";
    let mut socket = connect_to_spi_daemon(uri).unwrap();
    let rb = RingBuffer::new(2000);
    let (mut prod, mut cons) = rb.split();

    let model = Model {
        message_count: 0,
        incoming: cons
    };

    thread::spawn(move || {
        loop {
            let mut buffer = Vec::new();
            match socket.read_to_end(&mut buffer) {
                Ok(_) => {
                    match messageparsing::parse_message(&buffer) {
                        Ok(messages) => {
                            for spi_message in messages {
                                prod.push(spi_message);
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
    model.message_count += model.incoming.pop_each(
        |message| -> bool {
        return true;
    }, None) as u64;
}

fn view(app: &App, model: &Model, frame: Frame){
    let draw = app.draw();
    frame.clear(PURPLE);
    // We'll align to the window dimensions, but padded slightly.
    let win_rect = app.main_window().rect().pad(20.0);
    draw.background().color(PURPLE);

    let text = format!("message count: {}", model.message_count);

    draw.text(&text).color(WHITE).font_size(24).wh(win_rect.wh());

    draw.to_frame(app, &frame).unwrap();

}
