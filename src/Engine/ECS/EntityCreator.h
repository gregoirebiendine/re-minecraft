#ifndef FARFIELD_FACTORY_H
#define FARFIELD_FACTORY_H

#include <functional>
#include <string>
#include <unordered_map>

#include "IEntity.h"
#include "ISystem.h"
#include "World.h"
#include "Components/Camera.h"
#include "Components/CollisionBox.h"
#include "Components/Friction.h"
#include "Components/Gravity.h"
#include "Components/MeshRef.h"
#include "Components/Movements.h"
#include "Components/PlayerInput.h"
#include "Components/Inventory.h"

namespace ECS::Creator
{
    inline IEntity createPlayer(Handler& handler, const World& world)
    {
        // const auto mesh = world.getMeshRegistry().get("player");
        // const auto texture = world.getTextureRegistry().getByName("player");
        const auto player = handler.createEntity();

        handler.addComponent(player, Position{11.5f, 73.f, 11.5f});
        handler.addComponent(player, Velocity{});
        handler.addComponent(player, Rotation{});
        handler.addComponent(player, Camera{});
        handler.addComponent(player, PlayerInput{});
        handler.addComponent(player, Gravity{});
        handler.addComponent(player, Friction{});
        handler.addComponent(player, CollisionBox{{0.4f, 0.9f, 0.4f}});
        handler.addComponent(player, Hotbar{});
        handler.addComponent(player, InternalInventory{});
        handler.addComponent(player, Equipments{});
        // handler.addComponent(player, ECS::MeshRef{ mesh, texture });

        handler.getComponent<Hotbar>(player).items[0] = {1, Material::pack(world.getBlockRegistry().getByName("core:oak_log"), 0)};
        handler.getComponent<Hotbar>(player).items[1] = {1, Material::pack(world.getBlockRegistry().getByName("core:oak_plank"), 0)};
        handler.getComponent<Hotbar>(player).items[5] = {1, Material::pack(world.getBlockRegistry().getByName("core:coal_block"), 0)};
        handler.getComponent<Hotbar>(player).items[8] = {1, Material::pack(world.getBlockRegistry().getByName("core:furnace"), 0)};

        return player;
    }

    inline IEntity createZombie(Handler& handler, const World& world)
    {
        const auto mesh = world.getMeshRegistry().get("zombie");
        const auto texture = world.getTextureRegistry().getByName("zombie");
        const auto zombie = handler.createEntity();

        handler.addComponent(zombie, Position{7.5f, 73.f, 7.5f});
        handler.addComponent(zombie, Rotation{});
        handler.addComponent(zombie, Velocity{});
        handler.addComponent(zombie, Gravity{});
        handler.addComponent(zombie, CollisionBox{{0.45f, 1.f, 0.3f}});
        handler.addComponent(zombie, MeshRef{ mesh, texture });

        return zombie;
    }
}

#endif
