const firestoreService = require('firestore-export-import');
const fs = require('fs');

const serviceAccount = require("/home/jkao/Dropbox/projects/rfglow/firestore/bladeble-e714e-firebase-adminsdk-v93ee-43ec2a4dc3.json");
const databaseURL = "https://bladeble-e714e.firebaseio.com";

var collectionName = process.argv[2];

firestoreService.initializeApp(serviceAccount, databaseURL);

let result = firestoreService.backup(collectionName);

result.then(data => {
  fs.writeFile(collectionName+"-export.json", JSON.stringify(data),
    function(err) {
      if(err) {
          return console.log(err);
      }
      console.log("Saved collection to: "+collectionName+"-export.json");
    });
});

