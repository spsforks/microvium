import { BinaryRegion2, Future } from "../../lib/binary-region-2";
import { htmlTemplate, TestResults } from "../common";
import { binaryRegionFilenames } from "./filenames";

suite(BinaryRegion2.name, function () {
  test('empty', () => {
    const testResults = new TestResults();
    const region = new BinaryRegion2();

    const outputBinary = region.toBuffer();
    const outputHTML = htmlTemplate(region.toHTML());

    testResults.push(outputBinary, binaryRegionFilenames.empty.binary)
    testResults.push(outputHTML, binaryRegionFilenames.empty.html);

    testResults.checkAll();
  });

  test('basic', () => {
    const testResults = new TestResults();
    const region = new BinaryRegion2();

    region.writeUInt8(1);
    region.writeInt8(2);
    region.writeInt8(-2);
    region.writeUInt16LE(3);
    region.writeInt16LE(3);
    region.writeInt16LE(-3);
    region.writeUInt32LE(4);
    region.writeInt32LE(4);
    region.writeInt32LE(-4);
    region.writeDoubleLE(5);
    region.writeDoubleLE(0.5);
    region.writeDoubleLE(-0.5);
    region.writeStringUtf8NT('Hello, World!');

    const outputBinary = region.toBuffer();
    const outputHTML = htmlTemplate(region.toHTML());

    testResults.push(outputBinary, binaryRegionFilenames.basic.binary);
    testResults.push(outputHTML, binaryRegionFilenames.basic.html);

    testResults.checkAll();
  });

  test('futures', () => {
    const testResults = new TestResults();
    const region = new BinaryRegion2();

    const futurePrefilled = new Future();
    const futurePostFilled = new Future();
    const futureUnfilled = new Future();

    futurePostFilled.assign(Future.create(41));

    region.writeUInt8(1);
    region.writeInt8(2);
    region.writeInt8(-2);
    region.writeUInt16LE(3);
    region.writeInt16LE(futurePrefilled);
    region.writeInt16LE(futurePostFilled);
    region.writeInt16LE(futureUnfilled);
    region.writeUInt32LE(4);
    region.writeInt32LE(4);
    region.writeInt32LE(-4);
    region.writeDoubleLE(5);
    region.writeDoubleLE(0.5);
    region.writeDoubleLE(-0.5);
    region.writeStringUtf8NT('Hello, World!');

    futurePostFilled.assign(Future.create(42));

    const outputBinary = region.toBuffer();
    const outputHTML = htmlTemplate(region.toHTML());

    testResults.push(outputBinary, binaryRegionFilenames.futures.binary);
    testResults.push(outputHTML, binaryRegionFilenames.futures.html);

    testResults.checkAll();
  });
});