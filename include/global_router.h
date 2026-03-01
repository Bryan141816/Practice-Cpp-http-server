#pragma once
#include "router.h"

class GlobalRouter : public Router {
public:
  using Router::Router; 

  void includeRouter(const Router& child); 
};