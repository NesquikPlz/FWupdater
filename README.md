# FWupdater
bob 10th assignment - build secure FW updater
            
            
BOB10기 보안컨설팅 트랙 정시형멘토님 과제 : 보안에 안전한 펌웨어 업데이터 만들기
                        <br>  
FW를 구성하는 파일 3개(boot.tar, opt.tar, documentation.tar)는 용량문제로 못올림         
여기서 다운받으면 됨         
https://drive.google.com/drive/folders/1k5Zhtoml62OGA5u_E3H_XCcGo9k5dXt_?usp=sharing
                      <br>  <br>  


해당 펌웨어의 구조는 다음과 같음
<br>  
![image](https://user-images.githubusercontent.com/77532413/128472277-56b6745c-d5ca-4254-8904-51b52bc06a27.png)
<br>  
다음과 같은 환경을 가정하여 C언어로 하나의 firmware 파일을 argument로 받아서
firmware 내 파일들을 파일 시스템에 업데이트 하는 역할



우분투환경에서 실행하면 됨

컴파일 명령은 다음과 같음:                 
g++ updater.cpp -o updater -lssl -lcrypto -lstdc++    
에러뜨면 받으라는거 받으면 됨
<br>  

실행방법은 다음과 같음:             
./updater ./FW hihellonicetosee         

<br>  
여기서 hihellonicetosee는 128bit의 key 값임          
내가 그냥 임의로 설정함
시리얼코드 같이 사용자에게 주어졌다고 가정했음       

<br>  
정상적으로 실행되면 이렇게 뜬다
![image](https://user-images.githubusercontent.com/77532413/128475566-4386720f-1443-44ca-b063-a742d058088a.png)



수정하는 과정에서 캡처해서 파일이름이 좀 다르다 양해 부탁             
<br>  <br>  
그리고 /var/update_test/ 하위에 이런 구조가 생성됨

![image](https://user-images.githubusercontent.com/77532413/128473336-e38e63bd-77f0-4f26-a0ec-9e2dc03f2aa2.png)



해당 경로가 존재하지 않으면 에러뜸           
코드 열어서 경로를 바궈야 하는데 찾기 좀 어려울 수도 있음
아니면 저 경로에 저 폴더 만들어두면 됨
<br>  <br>                
*안전한* 업데이터를 위해서 내가 구현한 부분은 다음과 같음:            
     <br>                                                                                            
-사용자에게 시리얼 코드 입력 요청
-시리얼코드 길이 확인
-boot.tar와 opt.tar 파일 암호화           
-복호화 후 세 파일의 hash값 만들어서 업데이터 안에 저장되어있는 hash값과 비교          
<br>  <br>                                         
한계점은 다음과 같다 :             
<br>                  
-hash값이랑 key 값이 업데이터안에 평문으로 저장돼있어서 바이너리 파일 분석해서 키 값 가져올 수도 ㅜㅜ                






