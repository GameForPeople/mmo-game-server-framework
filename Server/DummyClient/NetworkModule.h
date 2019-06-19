#pragma once

void SendPacket(int cl, void *packet);
void InitializeNetwork();
void GetPointCloud(int *size, float **points);
