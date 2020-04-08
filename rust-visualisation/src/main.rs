use std::thread;
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
    let uri = "tcp://fpv-laptimer.local:5000";
    let mut socket = connect_to_spi_daemon(uri).unwrap();
    let handle = thread::spawn(move || {
        loop {
            let mut buffer = Vec::new();
            match socket.read_to_end(&mut buffer) {
                Ok(_) => {
                    println!("Read message {} bytes !", buffer.len());
                }
                Err(err) => {
                    panic!("Problem reading from socket: {}", err);
                }
            }
        }
    });    
    nannou::app(model)
        .update(update)
        .simple_window(view)
        .run();
}

struct Model {}

fn model(_app: &App) -> Model {
    Model {}
}

fn update(_app: &App, _model: &mut Model, _update: Update) {
}

fn view(_app: &App, _model: &Model, frame: Frame){
    frame.clear(PURPLE);
}
