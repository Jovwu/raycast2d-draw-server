import asyncio
import time
import websockets
import message_pb2
import pygame

pygame.init()
surface = pygame.Surface((640, 480))
screen = pygame.display.set_mode((640, 480), depth=8)


def get_client_message(event):
    # 这里判断是否会到达边界
    client_message = message_pb2.ClientMessage()
    client_message.ctl_flag = 1
    map2d_message = message_pb2.Map2DClientMessage()

    if event.key == pygame.K_UP:
        map2d_message.ctl_flag = message_pb2.Map2DClientMessage.ControlFlag.UP
    elif event.key == pygame.K_DOWN:
        map2d_message.ctl_flag = message_pb2.Map2DClientMessage.ControlFlag.DOWN
    elif event.key == pygame.K_LEFT:
        map2d_message.ctl_flag = message_pb2.Map2DClientMessage.ControlFlag.LEFT
    elif event.key == pygame.K_RIGHT:
        map2d_message.ctl_flag = message_pb2.Map2DClientMessage.ControlFlag.RIGHT

    client_message.for_map2d.CopyFrom(map2d_message)
    serialized_client_message = client_message.SerializeToString()
    return serialized_client_message


async def receive_messages(websocket):
    global surface
    while websocket.open:
        try:
            response = await websocket.recv()
        except websockets.ConnectionClosed:
            break
            
        server_message = message_pb2.ServerMessage()
        server_message.ParseFromString(response)

        canvas_list = list(server_message.for_map2d.canvas)
        for y in range(0, 480):
            for x in range(0, 640):
                surface.set_at((x, y), canvas_list[y * 640 + x])

        screen.blit(surface, (0, 0))
        pygame.display.flip()


async def send_keypress(websocket):
    while websocket.open:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                await websocket.close()
                while websocket.open:
                    time.sleep(0.1)
                return
            elif event.type == pygame.KEYDOWN:
                serialized_client_message = get_client_message(event)
                await websocket.send(serialized_client_message)

        await asyncio.sleep(0.01)


async def send_ping(websocket, interval):
    while websocket.open:
        await asyncio.sleep(interval) 
        if websocket.open:
            res = await websocket.ping()


async def connect_with_retry(uri, timeout, retries):
    for i in range(retries):
        try:
            return await asyncio.wait_for(websockets.connect(uri), timeout=timeout)
        except asyncio.TimeoutError:
            print(f"连接超时，正在进行第{i + 1}次重试...")
        except Exception as e:
            print(f"发生错误: {e}")
    raise Exception("连接失败，已达到最大重试次数")


async def main():
    timeout = 3  
    retries = 5  
    uri = "ws://192.168.1.131:8080" 
    websocket = await connect_with_retry(uri, timeout, retries)

    if websocket is None or not websocket.open:
        print("连接失败")
        return
    else:  # 连接成功
        print("连接成功")

    print(websocket.open)
    await asyncio.gather(
        send_keypress(websocket),
        receive_messages(websocket),
        send_ping(websocket, 10)
    )

asyncio.run(main())
pygame.quit()
exit(0)