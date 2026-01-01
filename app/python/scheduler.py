import json
import time
import threading

from modbus_engine import ModbusClientWrapper

class ModbusScheduler:
    def __init__(self, config_path):
        with open(config_path, 'r') as f:
            self.config = json.load(f)
            
        self.client = ModbusClientWrapper(
            self.config["port"],
            self.config["baudrate"],
            self.config["databits"],
            self.config["stopbits"],
            self.config["parity"],
            self.config["timeout_ms"],
            self.config["slave"]
        )
        
        self.interval = self.config["scan_interval_ms"] / 1000.0
        self.running = False
        self.thread = None
        
    def start(self):
        if self.running:
            return

        self.running = True
        self.thread = threading.Thread(target=self._loop, daemon=True)
        self.thread.start()
        
    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join()
        self.client.close()
        
    def _loop(self):
        start_time = time.time()
        
        while self.running:
            try:
                values = self.client.read_holding_registers(
                    self.config["start_address"],
                    self.config["quantity"]
                )
                
                timestamp = time.time()
                elapsed = (timestamp - start_time)
                
                # 여기서 다음 단계로 전달
                # - 로컬 저장
                # - 네트워크 전송
                print(f"Timestamp: {timestamp}, Elapsed: {elapsed}, Values: {values}")
            except Exception as e:
                print(f"Read error: {e}")
                
            time.sleep(self.interval)