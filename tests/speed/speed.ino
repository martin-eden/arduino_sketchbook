const uint32_t LoopLimit = 16000000;

void test_mul_ui1() {
  Serial.println("ui1");
  uint32_t StartTime = millis();

  uint8_t z = random(analogRead(A0));
  uint8_t c = analogRead(A0) + 7;
  for (uint32_t i = 0; i < LoopLimit; ++i) {
    z = z * c;
  }
  Serial.println(z);

  Serial.print("Done in ");
  Serial.print((float) (millis() - StartTime) / 1000, 3);
  Serial.println(" seconds.");
}

void test_mul_ui2() {
  Serial.println("ui2");
  uint32_t StartTime = millis();

  uint16_t z = random(analogRead(A0));
  uint16_t c = analogRead(A0) + 7;
  for (uint32_t i = 0; i < LoopLimit; ++i) {
    z = z * c;
  }
  Serial.println(z);

  Serial.print("Done in ");
  Serial.print((float) (millis() - StartTime) / 1000, 3);
  Serial.println(" seconds.");
}

void test_mul_ui4() {
  Serial.println("ui4");
  uint32_t StartTime = millis();

  uint32_t z = random(analogRead(A0));
  uint32_t c = analogRead(A0) + 7;
  for (uint32_t i = 0; i < LoopLimit; ++i) {
    z = z * c;
  }
  Serial.println(z);

  Serial.print("Done in ");
  Serial.print((float) (millis() - StartTime) / 1000, 3);
  Serial.println(" seconds.");
}

void test_mul_f8() {
  Serial.println("f8");
  uint32_t StartTime = millis();

  float z = random(analogRead(A0));
  float c = analogRead(A0) + 7;
  for (uint32_t i = 0; i < LoopLimit; ++i) {
    z = z * c;
  }
  Serial.println(z);

  Serial.print("Done in ");
  Serial.print((float) (millis() - StartTime) / 1000, 3);
  Serial.println(" seconds.");
}

void setup() {
  Serial.begin(9600);
  Serial.println("Arduino integer arithmetic speed test.");
}

void loop() {
  test_mul_ui1();
  test_mul_ui2();
  test_mul_ui4();
  test_mul_f8();
}