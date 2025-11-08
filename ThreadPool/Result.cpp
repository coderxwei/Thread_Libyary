#include "Result.h"
Result::Result(std::shared_ptr<Task> task, bool isValid) :is_valid_(isValid), task_(task)
{

}
