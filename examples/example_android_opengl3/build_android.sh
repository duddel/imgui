#!/bin/bash
cd android

cd gradle/wrapper
curl --location --output gradle-wrapper.jar.sha256 \
       https://services.gradle.org/distributions/gradle-5.4.1-wrapper.jar.sha256
echo "  gradle-wrapper.jar" >> gradle-wrapper.jar.sha256
sha256sum --check gradle-wrapper.jar.sha256
cd ../..

./gradlew assembleDebug
