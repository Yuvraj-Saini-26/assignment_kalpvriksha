#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"

#define TOTAL_TEAMS 10
#define MAX_NAME_LENGTH 50
#define MAX_PLAYERS_PER_TEAM 50
#define MAX_ID 1000
#define ROLE_BATSMAN 1
#define ROLE_BOWLER 2
#define ROLE_ALL_ROUNDER 3
#define NOT_FOUND -1

struct PlayerRecord {
    int id;
    char name[MAX_NAME_LENGTH + 1];
    int teamIndex;
    int role;
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;
    float performanceIndex;
};

struct PlayerNode {
    struct PlayerRecord *player;
    struct PlayerNode *next;
};

struct Team {
    int id;
    char name[MAX_NAME_LENGTH + 1];
    int totalPlayers;
    int battingPlayerCount;
    float strikeRateSum;
    struct PlayerNode *allPlayers;
    struct PlayerNode *roleLists[3];
};

struct Team teamList[TOTAL_TEAMS];

float calculatePerformanceIndex(int role, float battingAverage, float strikeRate, int wickets, float economyRate) {
    if (role == ROLE_BATSMAN) {
        return (battingAverage * strikeRate) / 100;
    } else if (role == ROLE_BOWLER) {
        return (wickets * 2) + (100 - economyRate);
    }

    return ((battingAverage * strikeRate) / 100) + (wickets * 2);
}

const char *getRoleName(int role) {
    if (role == ROLE_BATSMAN) {
        return "Batsman";
    } else if (role == ROLE_BOWLER) {
        return "Bowler";
    }

    return "All-Rounder";
}

const char *getRolePluralName(int role) {
    if (role == ROLE_BATSMAN) {
        return "Batsmen";
    } else if (role == ROLE_BOWLER) {
        return "Bowlers";
    }

    return "All-Rounders";
}

int roleFromText(const char *text) {
    if (strstr(text, "Bat") != NULL) {
        return ROLE_BATSMAN;
    } else if (strstr(text, "Bowl") != NULL) {
        return ROLE_BOWLER;
    }

    return ROLE_ALL_ROUNDER;
}

int findTeamIndex(int teamId) {
    int low = 0;
    int high = TOTAL_TEAMS - 1;

    while (low <= high) {
        int middle = (low + high) / 2;

        if (teamList[middle].id == teamId) {
            return middle;
        } else if (teamList[middle].id < teamId) {
            low = middle + 1;
        } else {
            high = middle - 1;
        }
    }

    return NOT_FOUND;
}

int isPlayerIdUsed(int id) {
    for (int teamNo = 0; teamNo < TOTAL_TEAMS; teamNo++) {
        struct PlayerNode *current = teamList[teamNo].allPlayers;

        while (current != NULL) {
            if (current->player->id == id) {
                return 1;
            }

            current = current->next;
        }
    }

    return 0;
}

struct PlayerNode *createNode(struct PlayerRecord *player) {
    struct PlayerNode *node = malloc(sizeof(struct PlayerNode));

    if (node == NULL) {
        printf("Error: Memory allocation failed.\n");
        exit(1);
    }

    node->player = player;
    node->next = NULL;
    return node;
}

struct PlayerNode *appendNode(struct PlayerNode *head, struct PlayerRecord *player) {
    struct PlayerNode *newNode = createNode(player);

    if (head == NULL) {
        return newNode;
    }

    struct PlayerNode *current = head;

    while (current->next != NULL) {
        current = current->next;
    }

    current->next = newNode;
    return head;
}

struct PlayerNode *insertSorted(struct PlayerNode *head, struct PlayerRecord *player) {
    struct PlayerNode *newNode = createNode(player);

    if (head == NULL || player->performanceIndex > head->player->performanceIndex) {
        newNode->next = head;
        return newNode;
    }

    struct PlayerNode *current = head;

    while (current->next != NULL && current->next->player->performanceIndex >= player->performanceIndex) {
        current = current->next;
    }

    newNode->next = current->next;
    current->next = newNode;
    return head;
}

void addPlayerToTeam(int teamIndex, struct PlayerRecord *player) {
    int roleIndex = player->role - 1;

    teamList[teamIndex].allPlayers = appendNode(teamList[teamIndex].allPlayers, player);
    teamList[teamIndex].roleLists[roleIndex] = insertSorted(teamList[teamIndex].roleLists[roleIndex], player);
    teamList[teamIndex].totalPlayers++;

    if (player->role != ROLE_BOWLER) {
        teamList[teamIndex].strikeRateSum += player->strikeRate;
        teamList[teamIndex].battingPlayerCount++;
    }
}

float getAverageStrikeRate(int teamIndex) {
    if (teamList[teamIndex].battingPlayerCount == 0) {
        return 0;
    }

    return teamList[teamIndex].strikeRateSum / teamList[teamIndex].battingPlayerCount;
}

void initializeTeams(void) {
    for (int teamNo = 0; teamNo < teamCount; teamNo++) {
        teamList[teamNo].id = teamNo + 1;
        strcpy(teamList[teamNo].name, teams[teamNo]);
        teamList[teamNo].totalPlayers = 0;
        teamList[teamNo].battingPlayerCount = 0;
        teamList[teamNo].strikeRateSum = 0;
        teamList[teamNo].allPlayers = NULL;
        teamList[teamNo].roleLists[0] = NULL;
        teamList[teamNo].roleLists[1] = NULL;
        teamList[teamNo].roleLists[2] = NULL;
    }
}

void loadInitialData(void) {
    for (int playerNo = 0; playerNo < playerCount; playerNo++) {
        int teamIndex = NOT_FOUND;

        for (int teamNo = 0; teamNo < TOTAL_TEAMS; teamNo++) {
            if (strcmp(teamList[teamNo].name, players[playerNo].team) == 0) {
                teamIndex = teamNo;
            }
        }

        if (teamIndex == NOT_FOUND) {
            continue;
        }

        struct PlayerRecord *player = malloc(sizeof(struct PlayerRecord));

        if (player == NULL) {
            printf("Error: Memory allocation failed.\n");
            exit(1);
        }

        player->id = players[playerNo].id;
        strcpy(player->name, players[playerNo].name);
        player->teamIndex = teamIndex;
        player->role = roleFromText(players[playerNo].role);
        player->totalRuns = players[playerNo].totalRuns;
        player->battingAverage = players[playerNo].battingAverage;
        player->strikeRate = players[playerNo].strikeRate;
        player->wickets = players[playerNo].wickets;
        player->economyRate = players[playerNo].economyRate;
        player->performanceIndex = calculatePerformanceIndex(player->role, player->battingAverage,
                                                             player->strikeRate, player->wickets,
                                                             player->economyRate);
        addPlayerToTeam(teamIndex, player);
    }
}

int readTeamId(const char *prompt) {
    int teamId;

    printf("%s", prompt);

    while (scanf("%d", &teamId) != 1 || teamId < 1 || teamId > MAX_ID) {
        printf("Invalid input!\n%s", prompt);
        while (getchar() != '\n');
    }

    return teamId;
}

int readRole(void) {
    int role;

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");

    while (scanf("%d", &role) != 1 || role < ROLE_BATSMAN || role > ROLE_ALL_ROUNDER) {
        printf("Invalid input!\nEnter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
        while (getchar() != '\n');
    }

    return role;
}

void printTableLine(void) {
    printf("====================================================================================\n");
}

void printPlayerRow(struct PlayerRecord *player, int showTeam) {
    printf("%-6d %-22s ", player->id, player->name);

    if (showTeam) {
        printf("%-14s ", teamList[player->teamIndex].name);
    }

    printf("%-12s %-7d %-7.1f %-7.1f %-6d %-6.1f %.2f\n", getRoleName(player->role), player->totalRuns,
           player->battingAverage, player->strikeRate, player->wickets, player->economyRate,
           player->performanceIndex);
}

void printTableHeader(int showTeam) {
    printTableLine();
    printf("%-6s %-22s ", "ID", "Name");

    if (showTeam) {
        printf("%-14s ", "Team");
    }

    printf("%-12s %-7s %-7s %-7s %-6s %-6s %s\n", "Role", "Runs", "Avg", "SR", "Wkts", "ER", "Perf.Index");
    printTableLine();
}

void addPlayer(void) {
    int teamId = readTeamId("Enter Team ID to add player: ");
    int teamIndex = findTeamIndex(teamId);

    if (teamIndex == NOT_FOUND) {
        printf("Error: Team not found.\n");
        return;
    }

    if (teamList[teamIndex].totalPlayers >= MAX_PLAYERS_PER_TEAM) {
        printf("Error: Team already has %d players.\n", MAX_PLAYERS_PER_TEAM);
        return;
    }

    int id;
    char name[MAX_NAME_LENGTH + 1];
    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;

    printf("Enter Player Details:\n");
    printf("Player ID: ");

    while (scanf("%d", &id) != 1 || id < 1 || id > MAX_ID) {
        printf("Invalid input!\nPlayer ID: ");
        while (getchar() != '\n');
    }

    if (isPlayerIdUsed(id)) {
        printf("Error: Player ID %d already exists.\n", id);
        return;
    }

    printf("Name: ");
    scanf(" %50[^\n]", name);
    while (getchar() != '\n');

    int role = readRole();

    printf("Total Runs: ");

    while (scanf("%d", &totalRuns) != 1 || totalRuns < 0) {
        printf("Invalid input!\nTotal Runs: ");
        while (getchar() != '\n');
    }

    printf("Batting Average: ");

    while (scanf("%f", &battingAverage) != 1 || battingAverage < 0) {
        printf("Invalid input!\nBatting Average: ");
        while (getchar() != '\n');
    }

    printf("Strike Rate: ");

    while (scanf("%f", &strikeRate) != 1 || strikeRate < 0) {
        printf("Invalid input!\nStrike Rate: ");
        while (getchar() != '\n');
    }

    printf("Wickets: ");

    while (scanf("%d", &wickets) != 1 || wickets < 0) {
        printf("Invalid input!\nWickets: ");
        while (getchar() != '\n');
    }

    printf("Economy Rate: ");

    while (scanf("%f", &economyRate) != 1 || economyRate < 0) {
        printf("Invalid input!\nEconomy Rate: ");
        while (getchar() != '\n');
    }

    struct PlayerRecord *player = malloc(sizeof(struct PlayerRecord));

    if (player == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    player->id = id;
    strcpy(player->name, name);
    player->teamIndex = teamIndex;
    player->role = role;
    player->totalRuns = totalRuns;
    player->battingAverage = battingAverage;
    player->strikeRate = strikeRate;
    player->wickets = wickets;
    player->economyRate = economyRate;
    player->performanceIndex = calculatePerformanceIndex(role, battingAverage, strikeRate, wickets, economyRate);

    addPlayerToTeam(teamIndex, player);
    printf("Player added successfully to Team %s!\n", teamList[teamIndex].name);
}

void displayTeamPlayers(void) {
    int teamId = readTeamId("Enter Team ID: ");
    int teamIndex = findTeamIndex(teamId);

    if (teamIndex == NOT_FOUND) {
        printf("Error: Team not found.\n");
        return;
    }

    printf("Players of Team %s:\n", teamList[teamIndex].name);
    printTableHeader(0);

    struct PlayerNode *current = teamList[teamIndex].allPlayers;

    while (current != NULL) {
        printPlayerRow(current->player, 0);
        current = current->next;
    }

    printTableLine();
    printf("Total Players: %d\n", teamList[teamIndex].totalPlayers);
    printf("Average Batting Strike Rate: %.2f\n", getAverageStrikeRate(teamIndex));
}

void displayTeamsByStrikeRate(void) {
    int order[TOTAL_TEAMS];

    for (int teamNo = 0; teamNo < TOTAL_TEAMS; teamNo++) {
        order[teamNo] = teamNo;
    }

    for (int position = 0; position < TOTAL_TEAMS - 1; position++) {
        int bestPosition = position;

        for (int nextPosition = position + 1; nextPosition < TOTAL_TEAMS; nextPosition++) {
            if (getAverageStrikeRate(order[nextPosition]) > getAverageStrikeRate(order[bestPosition])) {
                bestPosition = nextPosition;
            }
        }

        int temp = order[position];
        order[position] = order[bestPosition];
        order[bestPosition] = temp;
    }

    printf("Teams Sorted by Average Batting Strike Rate\n");
    printf("========================================================\n");
    printf("%-4s %-16s %-12s %s\n", "ID", "Team Name", "Avg Bat SR", "Total Players");
    printf("========================================================\n");

    for (int position = 0; position < TOTAL_TEAMS; position++) {
        int teamIndex = order[position];

        printf("%-4d %-16s %-12.1f %d\n", teamList[teamIndex].id, teamList[teamIndex].name,
               getAverageStrikeRate(teamIndex), teamList[teamIndex].totalPlayers);
    }

    printf("========================================================\n");
}

void displayTopPlayers(void) {
    int teamId = readTeamId("Enter Team ID: ");
    int teamIndex = findTeamIndex(teamId);

    if (teamIndex == NOT_FOUND) {
        printf("Error: Team not found.\n");
        return;
    }

    int role = readRole();
    int count;

    printf("Enter number of players: ");

    while (scanf("%d", &count) != 1 || count < 1) {
        printf("Invalid input!\nEnter number of players: ");
        while (getchar() != '\n');
    }

    struct PlayerNode *current = teamList[teamIndex].roleLists[role - 1];

    if (current == NULL) {
        printf("Team %s has no %s.\n", teamList[teamIndex].name, getRolePluralName(role));
        return;
    }

    printf("Top %d %s of Team %s:\n", count, getRolePluralName(role), teamList[teamIndex].name);
    printTableHeader(0);

    int printedCount = 0;

    while (current != NULL && printedCount < count) {
        printPlayerRow(current->player, 0);
        current = current->next;
        printedCount++;
    }

    printTableLine();

    if (printedCount < count) {
        printf("Only %d %s available in this team.\n", printedCount, getRolePluralName(role));
    }
}

struct PlayerNode *copyList(struct PlayerNode *head) {
    struct PlayerNode *copyHead = NULL;
    struct PlayerNode *copyTail = NULL;

    while (head != NULL) {
        struct PlayerNode *newNode = createNode(head->player);

        if (copyHead == NULL) {
            copyHead = newNode;
        } else {
            copyTail->next = newNode;
        }

        copyTail = newNode;
        head = head->next;
    }

    return copyHead;
}

struct PlayerNode *mergeTwoLists(struct PlayerNode *first, struct PlayerNode *second) {
    struct PlayerNode start;
    struct PlayerNode *tail = &start;

    start.next = NULL;

    while (first != NULL && second != NULL) {
        if (first->player->performanceIndex >= second->player->performanceIndex) {
            tail->next = first;
            first = first->next;
        } else {
            tail->next = second;
            second = second->next;
        }

        tail = tail->next;
    }

    if (first != NULL) {
        tail->next = first;
    } else {
        tail->next = second;
    }

    return start.next;
}

struct PlayerNode *mergeAllLists(struct PlayerNode *lists[], int low, int high) {
    if (low == high) {
        return lists[low];
    }

    int middle = (low + high) / 2;
    struct PlayerNode *leftList = mergeAllLists(lists, low, middle);
    struct PlayerNode *rightList = mergeAllLists(lists, middle + 1, high);

    return mergeTwoLists(leftList, rightList);
}

void freeNodes(struct PlayerNode *head) {
    while (head != NULL) {
        struct PlayerNode *nextNode = head->next;
        free(head);
        head = nextNode;
    }
}

void displayRolePlayersOfAllTeams(void) {
    int role = readRole();
    struct PlayerNode *extraLists[TOTAL_TEAMS];

    for (int teamNo = 0; teamNo < TOTAL_TEAMS; teamNo++) {
        extraLists[teamNo] = copyList(teamList[teamNo].roleLists[role - 1]);
    }

    struct PlayerNode *mergedList = mergeAllLists(extraLists, 0, TOTAL_TEAMS - 1);

    if (mergedList == NULL) {
        printf("No %s found in any team.\n", getRolePluralName(role));
        return;
    }

    printf("%s of all teams:\n", getRolePluralName(role));
    printTableHeader(1);

    struct PlayerNode *current = mergedList;

    while (current != NULL) {
        printPlayerRow(current->player, 1);
        current = current->next;
    }

    printTableLine();
    freeNodes(mergedList);
}

void freeEverything(void) {
    for (int teamNo = 0; teamNo < TOTAL_TEAMS; teamNo++) {
        freeNodes(teamList[teamNo].roleLists[0]);
        freeNodes(teamList[teamNo].roleLists[1]);
        freeNodes(teamList[teamNo].roleLists[2]);

        struct PlayerNode *current = teamList[teamNo].allPlayers;

        while (current != NULL) {
            struct PlayerNode *nextNode = current->next;
            free(current->player);
            free(current);
            current = nextNode;
        }
    }
}

void showMenu(void) {
    printf("\n==============================================================================\n");
    printf(" ICC ODI Player Performance Analyzer\n");
    printf("==============================================================================\n");
    printf("1. Add Player to Team\n");
    printf("2. Display Players of a Specific Team\n");
    printf("3. Display Teams by Average Batting Strike Rate\n");
    printf("4. Display Top K Players of a Specific Team by Role\n");
    printf("5. Display all Players of specific role Across All Teams by performance index\n");
    printf("6. Exit\n");
    printf("==============================================================================\n");
}

int main(void) {
    int choice = 0;

    initializeTeams();
    loadInitialData();

    do {
        showMenu();
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            while (getchar() != '\n');
            choice = 0;
            continue;
        }

        switch (choice) {
            case 1:
                addPlayer();
                break;
            case 2:
                displayTeamPlayers();
                break;
            case 3:
                displayTeamsByStrikeRate();
                break;
            case 4:
                displayTopPlayers();
                break;
            case 5:
                displayRolePlayersOfAllTeams();
                break;
            case 6:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid choice! Enter a number from 1 to 6.\n");
        }
    } while (choice != 6);

    freeEverything();
    return 0;
}
