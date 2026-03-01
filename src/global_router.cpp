#include "global_router.h"

void GlobalRouter::includeRouter(const Router &child)
{
  const auto &childRoutes = child.getAllRoute();
  routes_.insert(routes_.end(), childRoutes.begin(), childRoutes.end());
}