// Fill out your copyright notice in the Description page of Project Settings.


#include "GOAPPlanner.h"

#include "HLAction.h"
#include "LevelGenerator.h"
#include "Ship.h"

bool GOAPPlanner::Plan(AShip* Ship, bool bForwardSearch)
{

	Ship->PlannedActions.Empty();
	
	GOAPNode* GoalNode = new GOAPNode();
	TArray<TTuple<STATE_KEY, int, char>> GoalConditions = Ship->PickGoal();
	TMap<STATE_KEY, int> GoalState;
	for(TTuple<STATE_KEY, int, char> Condition : GoalConditions)
	{
		GoalState.Add(Condition.Get<0>(), Condition.Get<1>());
	}
	GoalNode->State = GoalState;
	GoalNode->Action = nullptr;
	GoalNode->RunningCost = 0;
	GoalNode->Parent = nullptr;

	
	GOAPNode* StartNode = new GOAPNode();
	StartNode->State = Ship->GetWorldState();
	StartNode->Action = nullptr;
	StartNode->RunningCost = 0;
	StartNode->Parent = nullptr;
	
	TArray<GOAPNode*> Open;
	TArray<GOAPNode*> Closed;

	if(bForwardSearch)
	{
		Open.Push(StartNode);
	}
	else
	{
		Open.Push(GoalNode);
	}

	//Feel free to change this if you want longer plans
	int MaxRunningCost = 10;

	while (Open.Num() > 0)
	{
		//float SmallestF = Open[0]->RunningCost + NodeHeuristic(StartNode->State, Open[0]->State, GoalConditions);
		float SmallestF = Open[0]->RunningCost + NodeHeuristic(GoalNode->State, Open[0]->State, GoalConditions);
		int SmallestFIndex = 0;
		
		for(int i = 1; i < Open.Num(); i++)
		{
			//int CurrentF = Open[i]->RunningCost + NodeHeuristic(StartNode->State, Open[i]->State, GoalConditions);
			int CurrentF = Open[i]->RunningCost + NodeHeuristic(GoalNode->State, Open[i]->State, GoalConditions);
			//if(CurrentF < SmallestF)
			if(CurrentF > SmallestF)
			{
				SmallestF = CurrentF;
				SmallestFIndex = i;
			}
		}
		
		GOAPNode* CurrentNode = Open[SmallestFIndex];
		Open.RemoveAt(SmallestFIndex);
		Closed.Add(CurrentNode);

		if(CurrentNode->RunningCost > MaxRunningCost)
		{
			return false;
		}

		//if(IsGoal(StartNode->State, CurrentNode->State,GoalConditions))
		if(IsGoal(GoalNode->State, CurrentNode->State,GoalConditions))
		{
			
			TArray<UHLAction*> ActionsToTake;
			//Start from Goal, then Goal - 1, all the way to Start
			//So ActionsToTake is Goal, Goal-1, etc.
			while(CurrentNode->Parent)
			{
				ActionsToTake.Add(CurrentNode->Action);
				CurrentNode = CurrentNode->Parent;
			}

			float BaseTime = Ship->LevelGenerator->TimePassed;
			// for (int i = 0; i < ActionsToTake.Num(); i++)
			for(int i = ActionsToTake.Num() - 1; i >= 0; i--)
			{
				Ship->PlannedActions.Add(ActionsToTake[i]);
				BaseTime = ActionsToTake[i]->OnActionConfirmed(Ship, BaseTime); // New Code
			}
			return true;
		}

		
		TArray<GOAPNode*> ConnectedNodes = Expand(CurrentNode, Ship);

		for(int i = 0; i < ConnectedNodes.Num(); i++)
		{
			int OpenTempTracker = 0;
			int ClosedTempTracker = 0;
			
			for(int j = 0; j < Open.Num(); j++)
			{
				if(!IsSameState(Open[j]->State, ConnectedNodes[i]->State))
				{
					OpenTempTracker++;
				}
			}
			for(int j = 0; j < Closed.Num(); j++)
			{
				if(!IsSameState(Closed[j]->State, ConnectedNodes[i]->State))
				{
					ClosedTempTracker++;
				}
			}
			if(ClosedTempTracker == Closed.Num())
			{
				int PossibleG = CurrentNode->RunningCost + ConnectedNodes[i]->Action->ActionCost;
				bool bPossibleGBetter = false;
				if(OpenTempTracker == Open.Num())
				{
					Open.Add(ConnectedNodes[i]);
					bPossibleGBetter = true;
				}
				else if(PossibleG < ConnectedNodes[i]->RunningCost)
				{
					bPossibleGBetter = true;
				}
				if(bPossibleGBetter)
				{
					ConnectedNodes[i]->Parent = CurrentNode;
					ConnectedNodes[i]->RunningCost = PossibleG;
				}
			}
		}
	}
	
	return false;
}

bool GOAPPlanner::IsGoal(TMap<STATE_KEY, int>& StartState, TMap<STATE_KEY, int>& CurrentState, TArray<TTuple<STATE_KEY, int, char>>& GoalConditions)
{
	for(TTuple<STATE_KEY, int, char> GoalCondition : GoalConditions)
	{
		if(CurrentState.Contains(GoalCondition.Get<0>()))
		{
			if(GoalCondition.Get<2>() == '>')
			{
				if(*CurrentState.Find(GoalCondition.Get<0>()) <= *StartState.Find(GoalCondition.Get<0>()))
				{
					return false;
				}
			}
			else if(GoalCondition.Get<2>() == '<')
			{
				if(*CurrentState.Find(GoalCondition.Get<0>()) > *StartState.Find(GoalCondition.Get<0>()))
				{
					return false;
				}
			}
			else if(GoalCondition.Get<2>() == '=')
			{
				if(*CurrentState.Find(GoalCondition.Get<0>()) != *StartState.Find(GoalCondition.Get<0>()))
				{
					return false;
				}
			}
		}
	}
	if(GoalConditions.IsEmpty())
	{
		return false;
	}
	return true;
}

bool GOAPPlanner::IsSameState(TMap<STATE_KEY, int>& Conditions, TMap<STATE_KEY, int>& State)
{
	for(TTuple<STATE_KEY, int> Condition : Conditions)
	{
		int* CurrentStateCondition = State.Find(Condition.Key);

		if(CurrentStateCondition)
		{
			if(Condition.Value != *CurrentStateCondition)
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

int GOAPPlanner::NodeHeuristic(TMap<STATE_KEY, int>& StartState, TMap<STATE_KEY, int>& State, TArray<TTuple<STATE_KEY, int, char>>& GoalConditions)
{
	int Heuristic = 0;
	for(TTuple<STATE_KEY, int, char> GoalCondition : GoalConditions)
	{
		if(StartState.Contains(GoalCondition.Get<0>()))
		{
			if(GoalCondition.Get<2>() == '<')
			{
				if(*State.Find(GoalCondition.Get<0>()) <= *StartState.Find(GoalCondition.Get<0>()))
				{
					Heuristic++;
				}
			}
			else if(GoalCondition.Get<2>() == '>')
			{
				if(*State.Find(GoalCondition.Get<0>()) > *StartState.Find(GoalCondition.Get<0>()))
				{
					Heuristic++;
				}
			}
			else if(GoalCondition.Get<2>() == '=')
			{
				if(*State.Find(GoalCondition.Get<0>()) != *StartState.Find(GoalCondition.Get<0>()))
				{
					Heuristic++;
				}
			}
		}
		//
		Heuristic += FMath::Abs(State[NumPoints] - StartState[NumPoints]);
		//
	}
	return Heuristic;
}

//---------------------------------------------- YOUR CODE HERE ----------------------------------------------//
TArray<GOAPNode*> GOAPPlanner::Expand(GOAPNode* Node, AShip* Ship)
{
	TArray<GOAPNode*> ConnectedNodes;

	for(int i = 0; i < Ship->AvailableActions.Num(); i++)
	{
		UHLAction* NewAction = NewObject<UHLAction>(Ship, Ship->AvailableActions[i]);
		if(NewAction->CheckPreconditions(Ship, Node->State))
		{
			if(!NewAction->SetupAction(Ship))
			{
				continue;
			}
			GOAPNode* NewNode = new GOAPNode();
			NewNode->State = Node->State; // TODO: Check if this is correct
			NewAction->ApplyEffects(Ship, NewNode->State);
			NewNode->Action = NewAction;
			NewNode->RunningCost = Node->RunningCost + NewAction->ActionCost;
			NewNode->Parent = Node;
			ConnectedNodes.Add(NewNode);
		}
	}
	
	return ConnectedNodes;
}