개발장치 구성
1.Raspberry PI 4 Model B
2.Raspberry PI Monitor
3.Mouse  & Keyboard
4.SD Card(64G / os : Raspbian Buster)
●
●
1. Download Iotivity Project  (1.3.0)
>> wget http://mirrors.kernel.org/iotivity/2.0.1.1/iotivity-2.0.1.1.tar.gz
>> tar zxvf iotivity-2.0.1.1.tar.gz
2.  Install the essential dependencies in Raspberry Pi box for building iotivity.
>> sudo apt-get update
>> sudo apt-get install scons build-essential libboost-dev libexpat1-dev libboost-thread-dev uuid-dev
>> sudo apt-get install \ git-core valgrind doxygen libtool autoconf
 \ libboost-all-dev libsqlite3-dev
 \ uuid-dev libglib2.0-dev libcurl4-gnutls-dev libbz2-dev
 
3. 외부라이브러리 설치
>> cd iotivity-2.0.1.1
git clone https://github.com/intel/tinycbor.git extlibs/tinycbor/tinycbor -b v0.5.1
git clone https://github.com/ARMmbed/mbedtls.git extlibs/mbedtls/mbedtls -b mbedtls-2.4.2
4. Build
>> Cd <iotivity_2.0.1.1>
>> scons TARGET_ARCH=arm SECURED=0
* 옵션을 안넣으면 OCPersistentSotrage 를 사용해야 put ,get 이 가능.
Example 실행 파일 위치
<iotivity_2.0.1.1>/out/linux/arm/release/examples
Example source code 위치
<iotivity_2.0.1.1>/resource/csd

