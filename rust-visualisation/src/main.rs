use std::thread;
use std::sync::mpsc;

use nannou::prelude::*;

use nanomsg::{Socket, Protocol, Error};
use std::io::{self, Read};


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
    incoming_rx: std::sync::mpsc::Receiver<u64>

}

fn model(_app: &App) -> Model {
    let uri = "tcp://fpv-laptimer.local:5000";
    let mut socket = connect_to_spi_daemon(uri).unwrap();
    let (tx, rx) = mpsc::channel();

    let handle = thread::spawn(move || {
        loop {
            let mut buffer = Vec::new();
            match socket.read_to_end(&mut buffer) {
                Ok(_) => {
                    println!("Read message {} bytes !", buffer.len());
                    tx.send(1);
                }
                Err(err) => {
                    panic!("Problem reading from socket: {}", err);
                }
            }
        }
    });

    Model {
        message_count: 0,
        incoming_rx: rx
    }
}

fn update(_app: &App, model: &mut Model, _update: Update) {
    match model.incoming_rx.try_recv()
    {
        Ok(number) => {     model.message_count += number; }
        Err(_) => {}
    }
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
