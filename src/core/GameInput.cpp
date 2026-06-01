#include "core/Game.h"


///////////////////////////////////////
/*               INPUT               */
///////////////////////////////////////

void Game::handleGameplayInput() {
    // player 1 - always local (ctrl 0)
    float p1Axis  = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTX, 0);
    bool p1Left   = isDown(options.keyP1Left)   || (p1Axis < -0.2f);
    bool p1Right  = isDown(options.keyP1Right)  || (p1Axis > 0.2f);
    bool p1Shield = isDown(options.keyP1Shield) || isDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, 0);

    if (player1.status != Status::DAMAGED) {
        player1.move((p1Left && p1Right) ? 0 : p1Left ? -1 : p1Right ? +1 : 0);
    }

    auto tryJumpWithParticles = [&](Player& player) {
        bool wasOnGround = player.onGround;
        bool hadAirJumped = player.hasAirJumped;
        player.jump();
        if (!wasOnGround && !hadAirJumped && player.hasAirJumped) {
            particles.spawnDoubleJump(player.rect.x + player.rect.w / 2.0f,
                                      player.rect.y + player.rect.h);
        }
    };

    if (input.jumpP1) {
        tryJumpWithParticles(player1);
        input.jumpP1 = false;
    }
    if (input.shootP1) {
        if (player1.tryShoot()) {
            int px = (player1.facing == Facing::LEFT) ? player1.rect.x - 20 : player1.rect.x + 20;
            projectiles.emplace_back(px, player1.rect.y, player1.facing, &player1);
        }
        input.shootP1 = false;
    }
    if (input.meleeP1) {
        if (player1.tryMelee()) {
            const int hw = 86, hh = 76;
            int hx = (player1.facing == Facing::RIGHT) ? player1.rect.x + player1.rect.w - 36 : player1.rect.x - hw + 36;
            int hy = player1.rect.y + (player1.rect.h - hh) / 2;
            meleeHitboxes.emplace_back(hx, hy, hw, hh, &player1, 6);
        }
        input.meleeP1 = false;
    }
    if (input.specialP1) {
        Direction dir = Direction::NONE;
        float axisX = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTX, 0);
        float axisY = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 0);
        if (axisX < -0.2f) dir = Direction::LEFT;
        else if (axisX > 0.2f) dir = Direction::RIGHT;
        else if (axisY < -0.2f) dir = Direction::UP;
        else if (axisY > 0.2f) dir = Direction::DOWN;
        if (isDown(options.keyP1Left)) dir = Direction::LEFT;
        else if (isDown(options.keyP1Right)) dir = Direction::RIGHT;
        else if (isDown(options.keyP1Jump)) dir = Direction::UP;
        else if (isDown(options.keyP1Down)) dir = Direction::DOWN;
        player1.trySpecial(dir);
        input.specialP1 = false;
    }

    player1.setShieldHeld(p1Shield);
    if (p1Shield) {
        player1.tryShield();
    } else if (!p1Shield && !player1.shieldHeld) {
        player1.releaseShield();
    }

    // player2 - local or remote
    bool p2Left, p2Right, p2Down, p2Shield;
    bool p2JumpPr, p2ShootPr, p2MeleePr, p2SpecialPr;
    bool p2JumpDn;

    if (networkMode == NetworkMode::REMOTE_HOST) {
        // remote player: read from network bits
        p2Left      = remoteIsDown(InputBit::LEFT);
        p2Right     = remoteIsDown(InputBit::RIGHT);
        p2Down      = remoteIsDown(InputBit::DOWN);
        p2Shield    = remoteIsDown(InputBit::SHIELD);
        p2JumpDn    = remoteIsDown(InputBit::JUMP);
        p2JumpPr    = remoteIsPressed(InputBit::JUMP);
        p2ShootPr   = remoteIsPressed(InputBit::SHOOT);
        p2MeleePr   = remoteIsPressed(InputBit::MELEE);
        p2SpecialPr = remoteIsPressed(InputBit::SPECIAL);
    } else {
        // local player 2: keyboard OR controller 1
        float p2Axis = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTX, 1);
        p2Left   = isDown(options.keyP2Left)   || (p2Axis < -0.2f);
        p2Right  = isDown(options.keyP2Right)  || (p2Axis > 0.2f);
        p2Down   = isDown(options.keyP2Down)   || getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 1) > 0.5f;
        p2Shield = isDown(options.keyP2Shield) || isDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER, 1);
        p2JumpDn = isDown(options.keyP2Jump)   || isDown(SDL_CONTROLLER_BUTTON_A, 1);
        p2JumpPr    = input.jumpP2;    input.jumpP2 = false;
        p2ShootPr   = input.shootP2;   input.shootP2 = false;
        p2MeleePr   = input.meleeP2;   input.meleeP2 = false;
        p2SpecialPr = input.specialP2; input.specialP2 = false;
    }

    if (player2.status != Status::DAMAGED)
        player2.move((p2Left && p2Right) ? 0 : p2Left ? -1 : p2Right ? 1 : 0);

    if (p2JumpPr) tryJumpWithParticles(player2);
    if (p2ShootPr) {
        if (player2.tryShoot()) {
            int px = (player2.facing == Facing::LEFT) ? player2.rect.x - 20 : player2.rect.x + 20;
            projectiles.emplace_back(px, player2.rect.y, player2.facing, &player2);
        }
    }
    if (p2MeleePr) {
        if (player2.tryMelee()) {
            const int hw = 86, hh = 76;
            int hx = (player2.facing == Facing::RIGHT) ? player2.rect.x + player2.rect.w - 36 : player2.rect.x - hw + 36;
            int hy = player2.rect.y + (player2.rect.h - hh) / 2;
            meleeHitboxes.emplace_back(hx, hy, hw, hh, &player2, 6);
        }
    }
    if (p2SpecialPr) {
        Direction dir = Direction::NONE;
        float p2AxisX = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTX, 1);
        float p2AxisY = getNormalizedAxis(SDL_CONTROLLER_AXIS_LEFTY, 1);
        if      (p2Left  || p2AxisX < -0.2f) dir = Direction::LEFT;
        else if (p2Right || p2AxisX >  0.2f) dir = Direction::RIGHT;
        else if (p2JumpDn || p2AxisY < -0.2f) dir = Direction::UP;
        else if (p2Down  || p2AxisY >  0.2f) dir = Direction::DOWN;
        player2.trySpecial(dir);
    }

    player2.setShieldHeld(p2Shield);
    if (p2Shield) {
        player2.tryShield();
    } else if (!p2Shield && !player2.shieldHeld) {
        player2.releaseShield();
    }
}

void Game::onKey(SDL_Keycode key, KeyAction action) {
    if (action != KeyAction::PRESS) return;
    if (key == options.keyQuit)       { running = false; return; }
    if (key == options.keyDebug)      { options.debug = !options.debug;  return; }
    if (key == options.keyFullscreen) {
        Uint32 flags = SDL_GetWindowFlags(window);
        SDL_SetWindowFullscreen(window,
            (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        return;
    }

    
    // handle play inputs

    if (key == options.keyP1Jump)    input.jumpP1    = true;
    if (key == options.keyP1Shoot)   input.shootP1   = true;
    if (key == options.keyP1Melee)   input.meleeP1   = true;
    if (key == options.keyP1Special) input.specialP1 = true;

    if (key == options.keyP2Jump)    input.jumpP2    = true;
    if (key == options.keyP2Shoot)   input.shootP2   = true;
    if (key == options.keyP2Melee)   input.meleeP2   = true;
    if (key == options.keyP2Special) input.specialP2 = true;
}

void Game::onControllerButton(SDL_GameControllerButton button, ControllerButtonAction action, int ctrl) {
    if (action != ControllerButtonAction::PRESS) return;

    // ctrl 0: P1 actions; ctrl 1: P2 actions (local only)
    if (ctrl == 0) {
        switch (button) {
            case SDL_CONTROLLER_BUTTON_A:     input.jumpP1    = true; break;
            case SDL_CONTROLLER_BUTTON_B:     input.shootP1   = true; break;
            case SDL_CONTROLLER_BUTTON_X:     input.meleeP1   = true; break;
            case SDL_CONTROLLER_BUTTON_Y:     input.specialP1 = true; break;
            default: break;
        }
    } else if (ctrl == 1 && networkMode != NetworkMode::REMOTE_CLIENT) {
        // P2 is only local when we're not the network client
        switch (button) {
            case SDL_CONTROLLER_BUTTON_A:     input.jumpP2    = true; break;
            case SDL_CONTROLLER_BUTTON_B:     input.shootP2   = true; break;
            case SDL_CONTROLLER_BUTTON_X:     input.meleeP2   = true; break;
            case SDL_CONTROLLER_BUTTON_Y:     input.specialP2 = true; break;
            default: break;
        }
    }
}

void Game::injectNavigationKey(SDL_KeyCode key) {
    if (!screens) return;
    Uint32 now = SDL_GetTicks();

    // if it's a different key than last time, reset the repeat state
    if (key != navRepeat.lastKey) {
        navRepeat.lastKey = key;
        navRepeat.lastTime = now;
        navRepeat.repeatActive = false;

        // always inject the first press immediately
        SDL_Event fake{};
        fake.type = SDL_KEYDOWN;
        fake.key.keysym.sym = key;
        fake.key.keysym.scancode = SDL_GetScancodeFromKey(key);
        screens.handle(fake);
        return;
    }

    // same key as before
    if (!navRepeat.repeatActive) {
        // wait for initial delay before allowing repeats
        if (now - navRepeat.lastTime >= NAV_INITIAL_DELAY) {
            navRepeat.repeatActive = true;
            navRepeat.lastTime = now;
            // inject the first repeat immediately
            SDL_Event fake{};
            fake.type = SDL_KEYDOWN;
            fake.key.keysym.sym = key;
            fake.key.keysym.scancode = SDL_GetScancodeFromKey(key);
            screens.handle(fake);
        }
    } else {
        // wait for repeat interval (since repeat is active)
        if (now - navRepeat.lastTime >= NAV_REPEAT_INTERVAL) {
            navRepeat.lastTime = now;
            SDL_Event fake{};
            fake.type = SDL_KEYDOWN;
            fake.key.keysym.sym = key;
            fake.key.keysym.scancode = SDL_GetScancodeFromKey(key);
            screens.handle(fake);
        }
    }
}

void Game::injectControllerNav(SDL_Event e) {
    if (e.type == SDL_CONTROLLERBUTTONDOWN) {
        SDL_KeyCode navKey = SDLK_UNKNOWN;
        switch (static_cast<SDL_GameControllerButton>(e.cbutton.button)) {
            case SDL_CONTROLLER_BUTTON_DPAD_UP:    navKey = SDLK_UP;     break;
            case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  navKey = SDLK_DOWN;   break;
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  navKey = SDLK_LEFT;   break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: navKey = SDLK_RIGHT;  break;
            case SDL_CONTROLLER_BUTTON_A:
            case SDL_CONTROLLER_BUTTON_START:      navKey = SDLK_RETURN; break;
            case SDL_CONTROLLER_BUTTON_B:
            case SDL_CONTROLLER_BUTTON_BACK:       navKey = SDLK_ESCAPE; break;
            default: break;
        }
        if (navKey != SDLK_UNKNOWN) {
            lastActiveNavAxis = SDL_CONTROLLER_AXIS_INVALID;  // reset stick tracking
            injectNavigationKey(navKey);
        }
    }

    if (e.type == SDL_CONTROLLERAXISMOTION) {
        SDL_GameControllerAxis axis = static_cast<SDL_GameControllerAxis>(e.caxis.axis);
        if (axis == SDL_CONTROLLER_AXIS_LEFTX || axis == SDL_CONTROLLER_AXIS_LEFTY) {
            float n = getNormalizedAxis(axis);
            SDL_KeyCode navKey = SDLK_UNKNOWN;

            if (axis == SDL_CONTROLLER_AXIS_LEFTY) {
                if (n > 0.2f)       navKey = SDLK_DOWN;
                else if (n < -0.2f) navKey = SDLK_UP;
            } else if (axis == SDL_CONTROLLER_AXIS_LEFTX) {
                if (n > 0.2f)       navKey = SDLK_RIGHT;
                else if (n < -0.2f) navKey = SDLK_LEFT;
            }

            if (navKey != SDLK_UNKNOWN) {
                lastActiveNavAxis = axis;
                injectNavigationKey(navKey);
            } else {
                // only reset if this axis was the one tracked
                if (axis == lastActiveNavAxis) {
                    navRepeat.lastKey = SDLK_UNKNOWN;
                    navRepeat.repeatActive = false;
                    lastActiveNavAxis = SDL_CONTROLLER_AXIS_INVALID;
                }
            }
        }
    }
}
