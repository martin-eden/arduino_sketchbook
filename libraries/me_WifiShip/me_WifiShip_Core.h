// WifiShip core.

/*
  Status: good
  Version: 8
  Last mod.: 2024-01-07
*/

/*
  This module provides functionality to get/set ship identity.

  Identity is Name (SSID) and Id (MAC).

  Design

    Init(): bool

    --( Id )--
    GetShipId(): bool, Id
    SetShipId(Id): bool

    --( Name )--
    GetShipName(): bool, Name
    SetShipName(Name): bool
*/

#pragma once

#include <me_Types.h>

#include "me_WifiShip_Common_CraftIdentity.h"

namespace me_WifiShip_Core
{
  /*
    Import Craft.. Name and Id types from CraftIdentity.h and rebrand
    them under Ship.. prefix.
  */
  // (
  const TUint_1 TShipId_Size = me_WifiShip_Common_CraftIdentity::TCraftId_Size;
  typedef me_WifiShip_Common_CraftIdentity::TCraftId TShipId;

  const TUint_1 TShipName_Size = me_WifiShip_Common_CraftIdentity::TCraftName_Size;
  typedef me_WifiShip_Common_CraftIdentity::TCraftName TShipName;
  // )

  // Core module:
  class TWifiShip_Core
  {
    public:
      TBool Init();

      TBool GetShipId(TShipId* ShipId);
      TBool SetShipId(TShipId ShipId);

      TBool GetShipName(TShipName* ShipName);
      TBool SetShipName(TShipName ShipName);
  };
}

/*
  2023-11-07
  2023-11-13
  2023-11-14
  2023-12-28 -- code structural design, network scan
  2023-12-31 -- using <me_Types.h>
  2024-01-01 -- splat to three modules: ship = (frame, scanner, docker)
  2024-01-03
  2024-02-07
*/