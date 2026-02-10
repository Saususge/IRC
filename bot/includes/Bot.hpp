#include <string>
#include <vector>

struct Message {
  std::string prefix;
  std::string command;
  std::vector<std::string> params;
};

Message parse_line(std::string line) {
  Message msg;

  if (line.empty()) return msg;

  // 2. ':' (Prefix) 처리 로직 (인덱스 0번 체크)
  // ... (여기에 작성)

  // 3. 공백 기준으로 자르면서 Command와 Params 채우기
  //    단, 중간에 ':' 만나면 뒤는 통으로 넣기 (loop)
  // ... (여기에 작성)

  return msg;
}