use std::sync::{Arc, Mutex};
use serde::{Deserialize, Serialize};
use tauri::{Emitter, State};
use tokio::io::{AsyncReadExt};
use tokio::net::TcpStream;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct TouchData {
    pub index: Option<usize>,
    pub id: i32,
    pub x: f64,
    pub y: f64,
    pub time: u64,
}

type TouchDataState = Arc<Mutex<Vec<String>>>;

// fn parse_touch_data(data: &str) -> Option<TouchData> {
//     let data = data.trim();
//     if data.len() < 14 { // Minimum length: 1+5+4+4+1 = 15, but we need at least 14
//         return None;
//     }
    
//     // Find where the fixed-width fields start by working backwards
//     // Format: %d%05d%04d%04d%lld
//     // Last field (time) is variable length, so we work from the end
//     let time_start = if data.len() >= 13 {
//         data.len() - 13 // Assuming minimum time length, but this needs adjustment
//     } else {
//         return None;
//     };
    
//     // For a more robust parsing, let's assume the format is:
//     // id + 5 digits (x) + 4 digits (y) + 4 digits + variable time
//     // We'll parse from right to left for the fixed parts
    
//     if data.len() < 14 {
//         return None;
//     }
    
//     // Extract time (last part, variable length)
//     let time_and_fixed_part = &data[data.len()-13..]; // Take last 13 chars for parsing backwards
    
//     // Try to find a reasonable split point
//     // Let's assume id is 1-3 digits, so fixed part starts after id
//     let mut id_end = 0;
//     for (i, c) in data.char_indices() {
//         if i > 0 && (i == 1 || i == 2 || i == 3) {
//             // Check if next 13 chars could be our fixed format
//             if data.len() >= i + 13 {
//                 id_end = i;
//                 break;
//             }
//         }
//     }

//     if id_end == 0 || data.len() < id_end + 13 {
//         return None;
//     }

//     let id_str = &data[..id_end];
//     let remaining = &data[id_end..];

//     if remaining.len() < 13 {
//         return None;
//     }

//     let x_str = &remaining[0..5];   // 5 digits
//     let y_str = &remaining[5..9];   // 4 digits  
//     let _extra_str = &remaining[9..13]; // 4 digits (unused)
//     let time_str = &remaining[13..]; // remaining for time

//     if let (Ok(id), Ok(x), Ok(y), Ok(time)) = (
//         id_str.parse::<i32>(),
//         x_str.parse::<f64>(),
//         y_str.parse::<f64>(),
//         time_str.parse::<u64>(),
//     ) {
//         return Some(TouchData {
//             index: None,
//             id,
//             x,
//             y,
//             time,
//         });
//     }
    
//     None
// }

#[tauri::command]
async fn start_tcp_connection(
    app_handle: tauri::AppHandle,
    state: State<'_, TouchDataState>,
) -> Result<bool, String> {
    let state_clone = state.inner().clone();
    let app_handle_clone = app_handle.clone();

    tokio::spawn(async move {
        match TcpStream::connect("localhost:8081").await {
            Ok(mut stream) => {
                app_handle_clone.emit("tcp-connected", true);
                println!("TCP bağlantısı kuruldu: localhost:8081");
                let mut buffer = [0; 1024];

                loop {
                    match stream.read(&mut buffer).await {
                        Ok(0) => {
                            println!("TCP bağlantısı kapandı");
                            let _ = app_handle_clone.emit("tcp-error", "Bağlantı kapandı");
                            break;
                        }
                        Ok(n) => {
                            let data = String::from_utf8_lossy(&buffer[..n]);
                            // println!("TCP: {}", data);
                            // println!("Gelen veri: '{}'", data);
                            let _ = app_handle_clone.emit("tcp-data", data);
                            // // Parse the incoming data into TouchData
                            // if let Some(touch_data) = parse_touch_data(&data) {
                            //     // Update state
                            //     if let Ok(mut state) = state_clone.lock() {
                            //         state.push(touch_data.clone());
                            //     }
                            //     // Emit TouchData object instead of raw string
                            //     let _ = app_handle_clone.emit("tcp-data", touch_data);
                            // } else {
                            //     println!("Veri parse edilemedi: {}", data);
                            // }
                        }
                        Err(e) => {
                            eprintln!("TCP okuma hatası: {}", e);
                            let _ = app_handle_clone.emit("tcp-error", format!("Okuma hatası: {}", e));
                            break;
                        }
                    }
                }
                return false;
            }
            Err(e) => {
                eprintln!("TCP bağlantı hatası: {}", e);
                let _ = app_handle_clone.emit("tcp-error", format!("Bağlantı hatası: {}", e));
                return false;
            }
        }
    });

    Ok(true)
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    let touch_data_state: TouchDataState = Arc::new(Mutex::new(Vec::new()));

    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .manage(touch_data_state)
        .invoke_handler(tauri::generate_handler![start_tcp_connection])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
