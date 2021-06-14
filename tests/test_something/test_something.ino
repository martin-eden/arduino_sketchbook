void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print(TCNT1H);
  Serial.print(" ");
  Serial.print(TCNT1L);
  Serial.print(" ");
  Serial.print(TCNT1L);
  Serial.println();
}
