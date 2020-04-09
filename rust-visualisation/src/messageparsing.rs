use std::convert::TryInto;

#[derive(Debug, Copy, Clone)]
pub struct Datagram {
    control: u32,
    payload: [u32; 8]
}

#[derive(Debug, Copy, Clone)]
pub struct Statistics {
    max_diff: u32,
    spi_max_diff: u32
}

pub union SpiMessage {
    data: Datagram,
    stats: Statistics
}


#[derive(Debug)]
pub enum Error {
    UnknownFormat,
    UnknownTypeCharacter
}

pub type Result = std::result::Result<SpiMessage, Error>;

pub fn parse_message(message: &Vec<u8>) -> std::result::Result<Vec<SpiMessage>, Error>
{
    let mut res = Vec::new();
    for part in message.split(|c| c == &(10 as u8)) {
        let line = std::str::from_utf8(part).unwrap().trim();
        if line.len() > 0 {
            match parse_line(line) {
                Ok(spi_message) => { res.push(spi_message); }
                Err(err) => { return Err(err) }
            }
        }
    }
    return Ok(res);
}


fn parse_line(message: &str) -> Result
{
    // A single Datagram looks like this:
    // D00000008:845274c1:00000091:84527e21:845287a1:84529121:84529aa1:8452a421:8452ada1

    // ^- type
    //         ^- valid data in this line, and possible excess data beyond
    //           ^- timestamp (propeller clocks)
    //                    ^- first ADC value

    let mut parts: Vec<&str> = message.split(":").collect();
    if parts.len() > 1 {
        let payload = parts.split_off(1);
        let command = parts[0];
        match &command[..1] {
            "D" => { return parse_datagram(command, &payload) }
            "S" => { return parse_stats(command, &payload) }
            _   => { return Err(Error::UnknownTypeCharacter) }
        }
    }
    return Err(Error::UnknownFormat);
}

fn parse_stats(_command: &str, _payload: &Vec<&str>) -> Result {
    let stats = Statistics{ max_diff: 0, spi_max_diff: 0 };
    let msg = SpiMessage{ stats: stats };
    return Ok(msg);
}


fn parse_datagram(_command: &str, payload: &Vec<&str>) -> Result {
    let payload: Vec<u32> = payload.into_iter().map(|v|  u32::from_str_radix(v, 16).unwrap()).collect();
    match payload.as_slice().try_into() {
        Ok(payload) => {
            let data = Datagram{
                control: 0,
                payload: payload
            };
            return Ok(SpiMessage{ data: data });
        }
        Err(_) => return Err(Error::UnknownFormat)
    }
}
