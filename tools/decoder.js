function toNumber(bytes) {
  var bits = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | (bytes[0]);
  var sign = ((bits >> 31) === 0) ? 1.0 : -1.0;
  var e = ((bits >> 23) & 0xff);
  var m = (e === 0) ? (bits & 0x7fffff) << 1 : (bits & 0x7fffff) | 0x800000;
  var f = sign * m * Math.pow(2, e - 150);
  return f;
}

function toInteger(bytes, len) {
  var out = 0;
  for (var i=len-1; i>=0; i--) {
    out = (out << 8) + bytes[i];
  }
  return out;
}

function toString(bytes) {
  var s = "";
  var i = 0;
  while (0 !== bytes[i]) {
    s = s + String.fromCharCode(bytes[i]);
    i++;
  }
  return s;
}

function toBool(bytes) {
  return (1 === bytes[0]);
}

function unpack(bytes) {

  // array to hold values
  var data = [];

  // first byte holds the number of elements
  var size = bytes[0];

  // get data types
  var types = [];
  var count = 1;
  do {
    var type = bytes[count];
    types.push(type >> 4);
    types.push(type & 0x0F);
    count++;
  } while (types.length < size);
  types = types.slice(0, size);

  // decode data
  for (var i=0; i<size; i++) {
    var type = types[i];
    if (0 === type) {
      data.push(toNumber(bytes.slice(count,count+4)));
      count += 4;
    } else if (1 === type) {
      data.push(toInteger(bytes.slice(count,count+4), 4));
      count += 4;
    } else if (5 === type) {
      data.push(toInteger(bytes.slice(count,count+2), 2));
      count += 2;
    } else if (6 === type) {
      data.push(toInteger(bytes.slice(count,count+1), 1));
      count += 1;
    } else if (3 === type) {
      data.push(toBool(bytes.slice(count,count+1)));
      count += 1;
    } else if (4 === type) {
      var s = toString(bytes.slice(count));
      data.push(s);
      count += (s.length + 1);
    }
  }

  return data;

}

// ----------------------------------------------------

function Decoder(bytes, port) {

  var decoded = {};

  // GPS info
  if (port == 10) {
    var gps = unpack(bytes);
    decoded.latitude = parseFloat(gps[0].toFixed(6));
    decoded.longitude = parseFloat(gps[1].toFixed(6));
    decoded.altitude = parseFloat(gps[2].toFixed(2));
    decoded.hdop = parseFloat(gps[3].toFixed(2));
    decoded.sats = parseInt(gps[4], 10);
  }

  return decoded;

}
