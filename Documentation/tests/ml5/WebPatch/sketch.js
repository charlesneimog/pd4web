let handPose;
let hands = [];
var p5jsConfigured = false;
let video;
let canvas;

function preload() {
  handPose = ml5.handPose({ flipped: true });
}

function gotHands(results) {
  hands = results;
}

function setup() {
  canvas = createCanvas(640, 480);
  canvas.parent("p5js");
  noLoop();
}

function startVideo() {
  video = createCapture(VIDEO, { flipped: true });
  video.hide();
  handPose.detectStart(video, gotHands);
  loop(); // Restart draw loop to start rendering video frames
}

function draw() {
  // Draw the webcam video
  image(video, 0, 0, width, height);

  // Draw rectangles in the corners
  fill(0, 255, 0, 100);
  rect(50, 0, 100, 100); // Top-left corner
  rect(width - 150, 0, 100, 100); // Top-right corner
  rect(50, height - 100, 100, 100); // Bottom-left corner
  rect(width - 150, height - 100, 100, 100); // Bottom-right corner

  let left_top_corner = false;
  let right_top_corner = false;
  let left_bottom_corner = false;
  let right_bottom_corner = false;

  // Draw all the tracked hand points
  for (let i = 0; i < hands.length; i++) {
    let hand = hands[i];
    for (let j = 0; j < hand.keypoints.length; j++) {
      let keypoint = hand.keypoints[j];
      fill(255, 0, 0);
      noStroke();
      circle(keypoint.x, keypoint.y, 5);

      if (Pd4Web) {
        let x = keypoint.x ;
        let y = keypoint.y;
        let name = keypoint.name;

        // check if it is the index_finger_tip
        if (name == "index_finger_tip" && Pd4Web.sendFloat){
            if (hand.hand == "left"){
              Pd4Web.sendFloat("metro", 500 + (1 - (y / height)) * 500);
            } 
        }


        // Check if the keypoint is inside any of the corners
        if (x >= 50 && x <= 150 && y >= 0 && y <= 100) {
          rect(50, 0, 100, 100); 

          let str = "record";
          fill(0, 0, 0);
          textSize(16);
          let textWidthValue = textWidth(str);
          let textHeightValue = textAscent() + textDescent();
          let xPos = 50 + (100 - textWidthValue) / 2;
          let yPos = (100 + textHeightValue) / 2 - textDescent();
          text(str, xPos, yPos);
          left_top_corner = true;   
          // Top-left corner
        } else if (x >= width - 150 && x <= width - 50 && y >= 0 && y <= 100) {
          rect(width - 150, 0, 100, 100); 
          
          let str = "granulador";
          fill(0, 0, 0);
          textSize(16);
          let textWidthValue = textWidth(str);
          let textHeightValue = textAscent() + textDescent();
          let xPos = width - 150 + (100 - textWidthValue) / 2;
          let yPos = (100 + textHeightValue) / 2 - textDescent();
          text(str, xPos, yPos);
          right_top_corner = true;
        } else if (x >= 50 && x <= 150 && y >= height - 100 && y <= height) {
          rect(50, height - 100, 100, 100);

          let str = "freeze";
          fill(0, 0, 0);
          textSize(16);
          let textWidthValue = textWidth(str);
          let textHeightValue = textAscent() + textDescent();
          let xPos = 50 + (100 - textWidthValue) / 2;
          let yPos = height - 100 + (100 + textHeightValue) / 2 - textDescent();
          text(str, xPos, yPos);
          bottom_left_corner = true;

          // Bottom-left corner
        } else if (x >= width - 150 && x <= width - 50 && y >= height - 100 && y <= height) {
          rect(width - 150, height - 100, 100, 100); // Bottom-right corner
          // Bottom-right corner
        }
      }
    }
  }

  if (Pd4Web){
    if (Pd4Web.sendList){
      if (left_top_corner){
        Pd4Web.sendFloat("record", 1);
      } else {
        Pd4Web.sendFloat("record", 0);
      }
      if (right_top_corner){
        Pd4Web.sendFloat("granulador", 1);
      } else {
        Pd4Web.sendFloat("granulador", 0);
      }
    }
  }
  

}
