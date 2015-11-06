var root;

Ionic.io();
// var user = Ionic.User.current();
// if (!user.id) {
//   user.id = Ionic.User.anonymousId();
// }
// user.save();

window.ionic.Platform.ready(function() {
  TestFairy.begin("e08d14885c1c81442f3329f9809554086920ff63");
  StatusBar.hide();
  angular.bootstrap(document, ['rfglow-ble']);
  blecontrol.initialize();
})

angular.module('rfglow-ble', ['ionic'])

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

.controller('RfglowCtrl', function($scope, $ionicModal) {
  console.log("Checking Ionic Deploy");
  var deploy = new Ionic.Deploy();

  deploy.check().then(function(isDeployAvailable) {
    // isDeployAvailable will be true if there is an update
    // and false otherwise
    if (isDeployAvailable) {
      console.log("New Ionic Deploy update is available!");
      deploy.update().then(function(deployResult) {
        // deployResult will be true when successfull and
        // false otherwise
      }, function(deployUpdateError) {
        // fired if we're unable to check for updates or if any
        // errors have occured.
      }, function(deployProgress) {
        // this is a progress callback, so it will be called a lot
        // deployProgress will be an Integer representing the current
        // completion percentage.
      });
    }
  }, function(deployCheckError) {
    // unable to check for deploy updates
    console.log("Error checking for Ionic Deploy update: "+deployCheckError);
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
    lastUpdateMs: 0
  }

  $scope.updateOff = function() {
    $scope.state.off = true;
    $scope.sendUpdate();
  }

  $scope.updateReset = function() {
    $scope.state.hue = 0;
    $scope.state.saturation = 255;
    $scope.state.brightness = 64;
    $scope.state.off = true;
    $scope.sendUpdate();
  }

  $scope.updateHS = function(h, s) {
    console.log("RFglowCtrl$scope updateHS");
    $scope.state.hue = h;
    $scope.state.saturation = s;
    $scope.state.off = false;
    $scope.sendUpdate();
  }

  $scope.updateHSV = function(h, s, v) {
    console.log("RFglowCtrl$scope updateHS");
    $scope.state.hue = h;
    $scope.state.saturation = s;
    $scope.state.brightness = v;
    $scope.state.off = false;
    $scope.sendUpdate();
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
