# linux-touchpad-driver

Threads:
- main(init, start tcp)
   - touchpad_thread
   - tcp_reader_thread (set_client üzerinden)
   - tcp_sender_thread (set_client üzerinden)