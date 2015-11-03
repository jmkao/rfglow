if (!window.blecontrol) {
  
  window.blecontrol = (function() {
    var blecontrol = {
      state: "Pre-init",
      device: null,
      easyble: null,
      $scope: function() {}, // set angularjs scope to this variable
      txUUID: "6e400002-b5a3-f393-e0a9-e50e24dcca9e",
      serviceUUID: "6e400001-b5a3-f393-e0a9-e50e24dcca9e",

      initialize: function () {
        console.log("Init blecontrol");
        this.state = "Initializing";
        this.$scope.$apply();
        this.easyble = evothings.easyble;
        this.easyble.closeConnectedDevices();
        this.easyble.reset();
        this.easyble.reportDeviceOnce(true);
        this.device = null;
        this.startScan();
      },

      // h, s, v = hue (0-359), saturation (0-255), brightness (0-255)
      writeHSV: function(h, s, v) {
        console.log("blecontrol.writeHSV()");
        if (this.device == null) {
          if (this.state == "Connected") {
            console.log("blecontrol.device is null even though state is connected. This should never happen!");
            this.initialize();
          } else {
            console.log("writeHSV() called while blecontrol not in Connected state. Resetting.");
            this.initialize();
          }
          return;
        }
      
        var blePayload = new Uint8Array(4);
        blePayload[0] = (h >> 8) & 0x00FF;
        blePayload[1] = h & 0x00FF;
        blePayload[2] = s;
        blePayload[3] = v;
        
        console.log("Calling device with payload: "+blePayload[0]+", "+blePayload[1]+", "+blePayload[2]+", "+blePayload[3]);
        this.device.writeCharacteristic(
          this.txUUID,
          blePayload,
          function() {
            console.log("writeHSV() successful");
          },
          function(errorCode) {
            console.log("writeHSV() failed with error: "+errorCode);
            console.log("Reinitializing...");
            blecontrol.initialize();
          }
        );
      },
    
      initDevice: function(device) {
        this.state = "Found device";
        this.$scope.$apply();
        device.connect(
          function(device) {
            blecontrol.state = "Polling device";
            blecontrol.$scope.$apply();
            device.readServices(
              [blecontrol.serviceUUID],
              function(device) {
                blecontrol.state = "Connected";
                blecontrol.$scope.$apply();
                blecontrol.device = device;
                console.log("Read services from device");
              },
              function(errorCode) {
                console.log("Failed to read services with error: "+errorCode);
                console.log("Reinitializing");
                blecontrol.initialize();
              }
            );
          },
          function(errorCode) {
            blecontrol.state = "Error";
            blecontrol.$scope.$apply();
            console.log("Connection to RFGLOW failed: " + errorCode);
            blecontrol.initialize();
          }
        )
      },
  
      startScan: function() {
        console.log("Scanning")
        this.state = "Scanning";
        this.$scope.$apply();
  
        this.easyble.startScan(
          function(device) {
            if (device != null && device.name != null) {
              console.log("Found device: "+device.name);
              if (device.name == 'RFGLOW') {
                blecontrol.easyble.stopScan();
                blecontrol.initDevice(device);
              }
            }
          },
          function(errorCode) {
            console.log("Error while scanning");
          }
        );
      },
    }
    return blecontrol;
  }());
}