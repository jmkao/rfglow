const firestoreService = require('firestore-export-import');

const serviceAccount = require("/home/jkao/Dropbox/projects/rfglow/firestore/bladeble-e714e-firebase-adminsdk-v93ee-43ec2a4dc3.json");
const databaseURL = "https://bladeble-e714e.firebaseio.com";

var restoreFile = process.argv[2];

firestoreService.initializeApp(serviceAccount, databaseURL);

let result = firestoreService.restore(restoreFile);
