var root;

Ionic.io();
// var user = Ionic.User.current();
// if (!user.id) {
//   user.id = Ionic.User.anonymousId();
// }
// user.save();

window.ionic.Platform.ready(function() {
  //window.cordova.plugins.Rollbar.init();

  angular.bootstrap(document, ['rfglow-ble']);

  if (!(typeof TestFairy === "undefined")) {
    console.log("TestFairy is present");
    //TestFairy.begin("e08d14885c1c81442f3329f9809554086920ff63");
  }

  if (!(typeof Rollbar === "undefined")) {
    console.log("Rollbar is present");
  }

  if (!(typeof StatusBar === "undefined")) {
    console.log("StatusBar is present");
    StatusBar.hide();
  }

  if (!(typeof evothings.ble === "undefined")) {
    console.log("EasyBLE is present");
    blecontrol.initialize();
  }

});

angular.module('rfglow-ble', ['ionic', 'ionic.cloud', 'tandibar/ng-rollbar'])

  .config(function($ionicCloudProvider) {
    $ionicCloudProvider.init({
      "core": {
        "app_id": "0ba1c21e"
      }
    });
  })

  .config(['RollbarProvider', function(RollbarProvider) {
      RollbarProvider.init({
      	accessToken: '47b865ea232f4c1389f6dbb30a3a4d7a',
          captureUncaught: true,
      	payload: {
              environment: 'rfglow-ble'
            }
          });
      console.log = Rollbar.info;
  }])


  .run(function($ionicPlatform, $location, $rootScope) {

    $ionicPlatform.ready(function() {
      // Hide the accessory bar by default (remove this to show the accessory bar above the keyboard
      // for form inputs)
      if(window.cordova && window.cordova.plugins.Keyboard) {
        cordova.plugins.Keyboard.hideKeyboardAccessoryBar(true);
      }
      if(window.StatusBar) {
        StatusBar.styleDefault();
      }
    });
  })

  .controller('RfglowCtrl', function($scope, $ionicModal, $ionicDeploy) {
    console.log("Checking Ionic Deploy");
    var deploy = $ionicDeploy;
    deploy.check().then(function(isDeployAvailable) {
      // isDeployAvailable will be true if there is an update
      // and false otherwise
      if (isDeployAvailable) {
        console.log("New Ionic Deploy update is available!");
        deploy.download().then(function() {
          console.log("New Ionic Deploy downloaded");
          deploy.extract().then(function () {
            console.log("New Ionic Deploy extracted");
            deploy.load();
          });
        });
      }
    });

  console.log("RfglowCtrl init");

  $scope.ble = blecontrol;
  blecontrol.$scope = $scope;
  console.log("State: "+$scope.ble.state);

  $ionicModal.fromTemplateUrl('templates/ble-popover.html', {
    scope: $scope,
    animation: 'slide-in-up'
  }).then(function(modal) {
    $scope.bleModal = modal;
    modal.show();
  });

  $scope.closeBLEModal = function () {
    $scope.bleModal.hide();
  };

  $scope.openBLEModal = function () {
    $scope.bleModal.show();
  }

  $scope.$watch('ble.state', function(newValue, oldValue) {
    if (newValue == "Connected" && oldValue != "Connected") {
      $scope.closeBLEModal();
    } else if (newValue != "Connected" && oldValue == "Connected") {
      $scope.openBLEModal();
    }
  });

  $scope.state = {
    hue: 0,
    saturation: 255,
    brightness: 64,
    prevH: 0,
    prevS: 255,
    prevB: 64,
    off: true,
    cycle: false,
    lastUpdateMs: 0
  }

  $scope.updateOff = function() {
    $scope.state.cycle = false;
    $scope.state.off = true;
    $scope.sendUpdate();
  }

  $scope.updateReset = function() {
    $scope.state.cycle = false;
    $scope.state.hue = 0;
    $scope.state.saturation = 255;
    $scope.state.brightness = 64;
    $scope.state.off = true;
    $scope.sendUpdate();
  }

  $scope.updateHS = function(h, s) {
    console.log("RFglowCtrl$scope updateHS");
    $scope.state.cycle = false;
    $scope.state.hue = h;
    $scope.state.saturation = s;
    $scope.state.off = false;
    $scope.sendUpdate();
  }

  $scope.updateHSV = function(h, s, v) {
    console.log("RFglowCtrl$scope updateHS");
    $scope.state.cycle = false;
    $scope.state.hue = h;
    $scope.state.saturation = s;
    $scope.state.brightness = v;
    $scope.state.off = false;
    $scope.sendUpdate();
  }

  $scope.cycleHS = function(hsArray, delayMs) {
    $scope.state.cycle = true;
    console.log("Cycle on");

    var counter = 0;
    var looper = function() {
      if ($scope.state.cycle) {
        var index = counter%hsArray.length;
        counter++;
        $scope.updateHS(hsArray[index][0], hsArray[index][1]);
        $scope.state.cycle = true;
        console.log("Cycle loop")

        setTimeout(looper, delayMs);
      } else {
        $scope.updateOff();
      }
    }

    looper();
  }

  $scope.sendUpdate = function() {
    var curMs = new Date().getTime();
    if (curMs - $scope.state.lastUpdateMs > 100) {
      //make sure we don't update too often

      if ($scope.state.off != true) {
        // we're not off
        blecontrol.writeHSV($scope.state.hue, $scope.state.saturation, $scope.state.brightness);
        $scope.state.prevH = $scope.state.hue;
        $scope.state.prevS = $scope.state.saturation;
        $scope.state.prevB = $scope.state.brightness;
        $scope.state.lastUpdateMs = curMs;
      } else if ($scope.state.off) {
        // we turned off, should have prev but won't flicker since it's off so laziness reigns
        blecontrol.writeHSV(0, 0, 0);
        $scope.state.lastUpdateMs = curMs;
      }
      // Otherwise, we didn't change state so don't do anything
    }
  }
});
