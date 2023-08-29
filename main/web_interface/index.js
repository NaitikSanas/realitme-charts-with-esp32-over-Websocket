document.addEventListener('DOMContentLoaded', function() {
var waveform1= {}, waveform2= {}, waveform3= {};
var waveform4= {}, waveform5= {}, waveform6= {};
let range = [0, 100];
let labels_data =  [];
let currentRetry = 0;
let maxRetries = 200;
var socket = null;


create_chart_obj_ctx(waveform1,"X-Angle", "X-Angle", range,false, true, labels_data , 0.1);
create_chart_obj_ctx(waveform2,"Y-Angle", "Y-Angle", range,false, true, labels_data , 0.1);
create_chart_obj_ctx(waveform3,"Z-Angle", "Z-Angle", range,false, true, labels_data , 0.1);

create_chart_obj_ctx(waveform4,"X-Acc", "X-Acceleration", range,false, true, labels_data , 0.1);
create_chart_obj_ctx(waveform5,"Y-Acc", "Y-Acceleration", range,false, true, labels_data , 0.1);
create_chart_obj_ctx(waveform6,"Z-Acc", "Z-Acceleration", range,false, true, labels_data , 0.1);

start_ws_client();



function start_ws_client(){
    var gateway = `ws://${window.location.hostname}/ws`;
    socket = new WebSocket(gateway);
    
    socket.onopen = function(event) {
      console.log("Connection  Opened");
        setInterval(function() {
            var message = 'data_request';
            socket.send(message);
        }, 20); 
    }

    socket.onclose = function(event) {
      if (currentRetry < maxRetries) {
        currentRetry++;
        console.log(`WebSocket connection closed. Retrying in 2 seconds... (Attempt ${currentRetry}/${maxRetries})`);
        setTimeout(start_ws_client, 500); // Retry after 2 seconds
      } else {
        console.error('WebSocket connection failed after maximum retry attempts.');
      }
    };
    
    

    socket.onmessage = function(event) {     
        try {
          var newData = JSON.parse(event.data);
          let lbl = [0, 75, 150, 225, 300, 375, 450, 525, 600];
          add_new_datapoints(waveform1.chart,newData.x, true, lbl);
          add_new_datapoints(waveform2.chart,newData.y, true, lbl);
          add_new_datapoints(waveform3.chart,newData.z, true, lbl);

          add_new_datapoints(waveform4.chart,newData.x_acc, true, lbl);
          add_new_datapoints(waveform5.chart,newData.y_acc, true, lbl);
          add_new_datapoints(waveform6.chart,newData.z_acc, true, lbl);
        
        } catch (error) {
          console.error("Error parsing JSON:", error);
          console.log("Received JSON:", event.data);
        }
        
    }
}

function add_new_datapoints(ctx, newData, appendLabels, labelsData) {
  const maxPoints = 96;

  if (appendLabels) {
    // Append new labels to the existing labels
    ctx.data.labels.push(...labelsData);
    // Ensure there are no more than 16 labels
    if (ctx.data.labels.length > maxPoints) {
      ctx.data.labels = ctx.data.labels.slice(-maxPoints);
    }
  }

  // Append new data points to the existing dataset
  ctx.data.datasets[0].data.push(...newData);
  // Ensure there are no more than 16 data points
  if (ctx.data.datasets[0].data.length > maxPoints) {
    ctx.data.datasets[0].data = ctx.data.datasets[0].data.slice(-maxPoints);
  }

  // Update the chart
  ctx.update({
    duration: 14,
    easing: 'linear',
    preservation: true
  });
}



function create_chart_obj_ctx(waveform_obj, elementid, chart_title_name, range, xaxis_display, yaxis_display, labels_data, line_tension) {
  var ctx1 = document.getElementById(elementid).getContext('2d');

  waveform_obj.chart = new Chart(ctx1, {
    type: 'line',
    data: {
      labels: labels_data,
      datasets: [
        {
          data: [],
          borderWidth: 1,
          borderColor: 'white',
          backgroundColor: 'rgba(0, 0, 0, 0)',
          pointRadius: 0,
          lineTension: line_tension,
          fill: false,
        },
        {
          data: [],
          borderWidth: 1,
          borderColor: 'white',
          backgroundColor: 'rgba(0, 0, 0, 0)',
          pointRadius: 0,
          lineTension: line_tension,
          fill: false,
        }
      ]
    },
    options: {
      layout: {
        padding: 40
      },
      width: 550,
      height: 350,
      responsive: false,
      backgroundColor: 'rgba(0, 0, 0, 1)',
      scales: {
        x: {
          type: 'category',
          offset: true,
          display: xaxis_display,
        },
        x2: {
          type: 'category',
          offset: true,
          display: false, // Set display to false to hide the x2-axis labels
        },
        y: {
          display: yaxis_display,
          grid: {
            color: 'rgba(255, 255, 255, 0.0)'
          },
          suggestedMin: range[0],
          suggestedMax: range[1]
        },

      },
      elements: {
        line: {
          backgroundColor: 'orange',
          borderColor: 'orange',
          tension: 0.3
        }
      },
      animation: {
        duration: 0
      },
      interaction: {
        mode: 'nearest',
        intersect: false
      },
      plugins: {
        title: {
          color: 'rgba(255, 255, 255, 1)',
          text: chart_title_name,
          display: true
        },
        legend: {
          display: false
        },
        streaming: {
          frameRate: 30,
          duration: 10000,
          delay: 0
        }
      }
    }
  });
}

});