const MAX_U32: i64 = 4294967296;

#[derive(Debug, Clone)]
pub struct PropellerTimeTracker
{
    cpu_freq: i64,
    timestamp_accu: i64,
    last_timestamp: Option<i64>
}


impl PropellerTimeTracker {

    pub fn new() -> PropellerTimeTracker {
        PropellerTimeTracker{ cpu_freq: 80_000_000, timestamp_accu: 0, last_timestamp: None }
    }

    pub fn feed(&mut self, timestamp: u32) -> f32 {
        let timestamp = timestamp as i64;
        match self.last_timestamp {
            Some(last_timestamp) => {
                let diff = (timestamp + MAX_U32 - last_timestamp) % MAX_U32;
                self.timestamp_accu += diff;
            }
            None => {}
        }
        self.last_timestamp = Some(timestamp);
        (self.timestamp_accu as f64 / self.cpu_freq as f64) as f32
    }
}
