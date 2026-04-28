## WIP: CXI transfer engine implementation
Currently the installation is ok, transfer engine is able to find the cxi devices,
since CXI only supports FI_MR_ENDPOINT for fi_info mr attributes, which means that a memory region can be bound to one endpoint only.
Unluckily, it's not possible to have a single MR bound to multiple EPs, which means that the current pattern used (one endpoint <==> one communication peer)
must be decoupled (otherwise we would have 1 MR <==> one communication peer, which could be acceptable depending on the use case). To do so the plan is:
- an endpoint pool at context instantation time (when we instantiate the NIC's domain). Since each endpoint can only be bound to 1 queue, we should have 
  32 endpoints per domain (per NIC). For a total of 32 * 4 = 128 endpoints for one node.
- Since 1 MR <==> 1 endpoint we either accept that one memory buffer can 