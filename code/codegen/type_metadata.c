
//~ helpers ///////////////////////////////////////////////////////////////////

MD_Node*
gen_get_child_value(MD_Node *parent, MD_String8 child_name)
{
 MD_Node *child = MD_ChildFromString(parent, child_name, 0);
 MD_Node *result = child->first_child;
 return(result);
}

GEN_TypeInfo*
gen_resolve_type_info_from_string(MD_String8 name)
{
 GEN_TypeInfo *result = 0;
 // @notes The MD_Map helper is a "flexibly" typed hash table. It's keys can
 //  be a mix of strings and pointers. Here MD_MapKeyStr(name) is making the
 //  `name` string into a key for the map. The lookup function returns a
 //  "map slot" because the map is not restricted to storing just one value
 //  per key, if we were using it that way we could use MD_MapScan to
 //  iterate through the map slots.
 MD_MapSlot *slot = MD_MapLookup(&type_map, MD_MapKeyStr(name));
 if (slot != 0)
 {
  result = (GEN_TypeInfo*)slot->val;
 }
 return(result);
}

GEN_TypeInfo*
gen_resolve_type_info_from_referencer(MD_Node *reference)
{
 GEN_TypeInfo *result = gen_resolve_type_info_from_string(reference->string);
 return(result);
}

GEN_TypeEnumerant*
gen_enumerant_from_name(GEN_TypeInfo *enum_type, MD_String8 name)
{
 GEN_TypeEnumerant *result = 0;
 for (GEN_TypeEnumerant *enumerant = enum_type->first_enumerant;
      enumerant != 0;
      enumerant = enumerant->next)
 {
  if (MD_S8Match(name, enumerant->node->string, 0))
  {
   result = enumerant;
   break;
  }
 }
 return(result);
}

GEN_MapCase*
gen_map_case_from_enumerant(GEN_MapInfo *map, GEN_TypeEnumerant *enumerant)
{
 GEN_MapCase *result = 0;
 for (GEN_MapCase *map_case = map->first_case;
      map_case != 0;
      map_case = map_case->next)
 {
  if (map_case->in_enumerant == enumerant)
  {
   result = map_case;
   break;
  }
 }
 return(result);
}

MD_Node*
gen_get_symbol_md_node_by_name(MD_String8 name)
{
 MD_Node *result = MD_NilNode();
 MD_MapSlot *type_slot = MD_MapLookup(&type_map, MD_MapKeyStr(name));
 if (type_slot != 0)
 {
  GEN_TypeInfo *type_info = (GEN_TypeInfo*)type_slot->val;
  result = type_info->node;
 }
 MD_MapSlot *map_slot = MD_MapLookup(&map_map, MD_MapKeyStr(name));
 if (map_slot != 0)
 {
  GEN_MapInfo *map_info = (GEN_MapInfo*)map_slot->val;
  result = map_info->node;
 }
 return(result);
}

void
gen_type_resolve_error(MD_Node *reference)
{
 MD_CodeLoc loc = MD_CodeLocFromNode(reference);
 MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                    "could not resolve type name '%.*s'", MD_S8VArg(reference->string));
}

void
gen_duplicate_symbol_error(MD_Node *new_node, MD_Node *existing_node)
{
 MD_CodeLoc loc = MD_CodeLocFromNode(new_node);
 MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                    "Symbol name '%.*s' is already used",
                    MD_S8VArg(new_node->string));
 MD_CodeLoc existing_loc = MD_CodeLocFromNode(existing_node); 
 MD_PrintMessageFmt(error_file, existing_loc, MD_MessageKind_Note,
                    "See '%.*s' is already used",
                    MD_S8VArg(existing_node->string));
}

void
gen_check_and_do_duplicate_symbol_error(MD_Node *new_node)
{
 MD_Node *existing = gen_get_symbol_md_node_by_name(new_node->string);
 if (!MD_NodeIsNil(existing))
 {
  gen_duplicate_symbol_error(new_node, existing);
 }
}


//~ analyzers /////////////////////////////////////////////////////////////////

// @notes The first stage of processing is to loop over the top level nodes
//  from each parse. We are using the tags `@type` and `@map` to mark the nodes
//  that this generator will process. Whenever we see one of those tags we
//  create a GEN_TypeInfo or GEN_MapInfo to gather up information from the
//  stages of analysis, and we insert the new info pointer into the appropriate
//  map. On the types we do a little bit of the analysis right in this function
//  to figure out which "type kind" it is, this lets us avoid ever having info
//  where the kind field is not one of the expected values.

void
gen_gather_types_and_maps(MD_Node *list)
{
 for(MD_EachNode(ref, list->first_child))
 {
  MD_Node *root = MD_ResolveNodeFromReference(ref);
  for(MD_EachNode(node, root->first_child))
  {
   // gather type
   MD_Node *type_tag =  MD_TagFromString(node, MD_S8Lit("type"), 0);
   
   if (!MD_NodeIsNil(type_tag))
   {
    gen_check_and_do_duplicate_symbol_error(node);
    
    GEN_TypeKind kind = GEN_TypeKind_Null;
    MD_Node   *tag_arg_node = type_tag->first_child;
    MD_String8 tag_arg_str = tag_arg_node->string;
    if (MD_S8Match(tag_arg_str, MD_S8Lit("basic"), 0))
    {
     kind = GEN_TypeKind_Basic;
    }
    else if (MD_S8Match(tag_arg_str, MD_S8Lit("struct"), 0))
    {
     kind = GEN_TypeKind_Struct;
    }
    else if (MD_S8Match(tag_arg_str, MD_S8Lit("enum"), 0))
    {
     kind = GEN_TypeKind_Enum;
    }
    else if (MD_S8Match(tag_arg_str, MD_S8Lit("ext"), 0))
    {
     kind = GEN_TypeKind_Ext;
    }
    else if (MD_S8Match(tag_arg_str, MD_S8Lit("list"), 0))
    {
     kind = GEN_TypeKind_ListType;
    }
    
    if (kind == GEN_TypeKind_Null)
    {
     MD_CodeLoc loc = MD_CodeLocFromNode(node);
     MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                        "Unrecognized type kind '%.*s'",
                        MD_S8VArg(tag_arg_str));
    }
    else
    {
     GEN_TypeInfo *type_info = MD_PushArrayZero(arena, GEN_TypeInfo, 1);
     type_info->kind = kind;
     type_info->node = node;
     MD_QueuePush(first_type, last_type, type_info);
     MD_MapInsert(arena, &type_map, MD_MapKeyStr(node->string), type_info);
    }
   }
   
   // gather map
   if (MD_NodeHasTag(node, MD_S8Lit("map"), 0))
   {
    gen_check_and_do_duplicate_symbol_error(node);
    
    GEN_MapInfo *map_info = MD_PushArrayZero(arena, GEN_MapInfo, 1);
    map_info->node = node;
    MD_QueuePush(first_map, last_map, map_info);
    MD_MapInsert(arena, &map_map, MD_MapKeyStr(node->string), map_info);
   }
  }
 }
}

void
gen_check_duplicate_member_names(void)
{
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  MD_Node *type_root_node = type->node;
  for (MD_EachNode(member_node, type_root_node->first_child))
  {
   MD_String8 name = member_node->string;
   for (MD_EachNode(check_node, type_root_node->first_child))
   {
    if (member_node == check_node)
    {
     break;
    }
    if (MD_S8Match(name, check_node->string, 0))
    {
     MD_CodeLoc my_loc = MD_CodeLocFromNode(member_node);
     MD_CodeLoc og_loc = MD_CodeLocFromNode(check_node);
     MD_PrintMessageFmt(error_file, my_loc, MD_MessageKind_Error,
                        "'%.*s' is already defined", MD_S8VArg(name));
     MD_PrintMessageFmt(error_file, og_loc, MD_MessageKind_Note,
                        "see previous definition of '%.*s'", MD_S8VArg(name));
     break;
    }
   }
  }
 }
}

// @notes In the next few stages of analysis we 'equip' the info nodes we 
//  gathered with further information by examining the sub-trees rooted at the 
//  metadesk nodes we saw durring the gather phase.

void
gen_equip_basic_type_size(void)
{
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  if (type->kind == GEN_TypeKind_Basic)
  {
   // extract the size
   int size = 0;
   
   MD_Node *size_node = type->node->first_child;
   MD_Node *error_at = 0;
   if (MD_NodeIsNil(size_node))
   {
    error_at = type->node;
   }
   else
   {
    MD_String8 size_string = size_node->string;
    if (!MD_StringIsCStyleInt(size_string))
    {
     error_at = size_node;
    }
    else
    {
     size = (int)MD_CStyleIntFromString(size_string);
    }
   }
   if (error_at != 0)
   {
    MD_CodeLoc loc = MD_CodeLocFromNode(error_at);
    MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                    MD_S8Lit("a basic type requires a plain integer size specifier"));
   }
   
   // save the size
   type->size = size;
  }
 }
}

void
gen_equip_struct_members(void)
{
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  if ((type->kind == GEN_TypeKind_Struct) || (type->kind == GEN_TypeKind_ListType))
  {
   // build the list
   MD_b32 got_list = 1;
   GEN_TypeMember *first_member = 0;
   GEN_TypeMember *last_member = 0;
   int member_count = 0;
   
   MD_Node *type_root_node = type->node;
   for (MD_EachNode(member_node, type_root_node->first_child))
   {
    MD_Node *type_name_node = member_node->first_child;
    
    // missing type node?
    if (MD_NodeIsNil(type_name_node))
    {
     MD_CodeLoc loc = MD_CodeLocFromNode(member_node);
     MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                     MD_S8Lit("Missing type name for member"));
     got_list = 0;
     goto skip_member;
    }
    
    // has type node:
    MD_String8 type_name = type_name_node->string;
    GEN_TypeInfo *type_info = gen_resolve_type_info_from_string(type_name);
    
    // could not resolve type?
    if (type_info == 0)
    {
     gen_type_resolve_error(type_name_node);
     got_list = 0;
     goto skip_member;
    }
    
    // resolved type:
    if (got_list)
    {
     GEN_TypeMember *array_count = 0;
     MD_Node *array_tag = MD_TagFromString(type_name_node, MD_S8Lit("array"), 0);
     if (!MD_NodeIsNil(array_tag))
     {
      MD_Node *array_count_referencer = array_tag->first_child;
      if (array_count_referencer->string.size == 0)
      {
       MD_CodeLoc loc = MD_CodeLocFromNode(array_tag);
       MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                       MD_S8Lit("array tags must specify a parameter for their count"));
      }
      else
      {
       MD_Node *array_count_member_node =
        MD_ChildFromString(type_root_node, array_count_referencer->string, 0);
       if (MD_NodeIsNil(array_count_member_node))
       {
        MD_CodeLoc loc = MD_CodeLocFromNode(array_count_referencer);
        MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                           "'%.*s' is not a member of %.*s",
                           MD_S8VArg(array_count_referencer->string), MD_S8VArg(type_name));
       }
       else
       {
        for (GEN_TypeMember *member_it = first_member;
             member_it != 0;
             member_it = member_it->next)
        {
         if (member_it->node == array_count_member_node)
         {
          array_count = member_it;
          break;
         }
        }
        if (array_count == 0)
        {
         MD_CodeLoc loc = MD_CodeLocFromNode(array_count_referencer);
         MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                            "'%.*s' comes after this array",
                            MD_S8VArg(array_count_referencer->string), MD_S8VArg(type_name));
        }
       }
      }
     }
     
     GEN_TypeMember *member = MD_PushArray(arena, GEN_TypeMember, 1);
     member->node = member_node;
     member->type = type_info;
     member->array_count = array_count;
     member->member_index = member_count;
     MD_QueuePush(first_member, last_member, member);
     member_count += 1;
    }
    
    skip_member:;
   }
   
   // save the list
   if (got_list)
   {
    type->first_member = first_member;
    type->last_member = last_member;
    type->member_count = member_count;
   }
  }
 }
}

void
gen_equip_enum_underlying_type(void)
{
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  if (type->kind == GEN_TypeKind_Enum)
  {
   // extract underlying type
   GEN_TypeInfo *underlying_type = 0;
   
   MD_Node *type_node = type->node;
   MD_Node *type_tag = MD_TagFromString(type_node, MD_S8Lit("type"), 0);
   MD_Node *type_tag_param = type_tag->first_child;
   MD_Node *underlying_type_ref = type_tag_param->first_child;
   if (!MD_NodeIsNil(underlying_type_ref))
   {
    GEN_TypeInfo *resolved_type = gen_resolve_type_info_from_referencer(underlying_type_ref);
    if (resolved_type == 0)
    {
     gen_type_resolve_error(underlying_type_ref);
    }
    else
    {
     if (resolved_type->kind != GEN_TypeKind_Basic)
     {
      MD_CodeLoc loc = MD_CodeLocFromNode(underlying_type_ref);
      MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                         "'%.*s' is not a basic type",
                         MD_S8VArg(underlying_type_ref->string));
     }
     else
     {
      underlying_type = resolved_type;
     }
    }
   }
   
   // save underlying type
   type->underlying_type = underlying_type;
  }
 }
}

void
gen_equip_enum_members(void)
{
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  if (type->kind == GEN_TypeKind_Enum)
  {
   
   // build the list
   MD_b32 got_list = 1;
   GEN_TypeEnumerant *first_enumerant = 0;
   GEN_TypeEnumerant *last_enumerant = 0;
   int enumerant_count = 0;
   
   int next_implicit_value = 0;
   
   MD_Node *type_root_node = type->node;
   for (MD_EachNode(enumerant_node, type_root_node->first_child))
   {
    MD_Node *value_node = enumerant_node->first_child;
    int value = 0;
    
    // missing value node?
    if (MD_NodeIsNil(value_node))
    {
     value = next_implicit_value;
     next_implicit_value += 1;
    }
    
    // has value node
    else
    {
     MD_String8 value_string = value_node->string;
     if (!MD_StringIsCStyleInt(value_string))
     {
      got_list = 0;
      goto skip_enumerant;
     }
     value = (int)MD_CStyleIntFromString(value_string);
    }
    
    // set next implicit value
    next_implicit_value = value + 1;
    
    // save enumerant
    if (got_list)
    {
     GEN_TypeEnumerant *enumerant = MD_PushArray(arena, GEN_TypeEnumerant, 1);
     enumerant->node = enumerant_node;
     enumerant->value = value;
     MD_QueuePush(first_enumerant, last_enumerant, enumerant);
     enumerant_count += 1;
    }
    
    skip_enumerant:;
   }
   
   // save the list
   if (got_list)
   {
    type->first_enumerant = first_enumerant;
    type->last_enumerant = last_enumerant;
    type->enumerant_count = enumerant_count;
   }
  }
 }
}

void
gen_equip_map_in_out_types(void)
{
 for (GEN_MapInfo *map = first_map;
      map != 0;
      map = map->next)
 {
  MD_Node *map_root_node = map->node;
  MD_Node *map_tag = MD_TagFromString(map_root_node, MD_S8Lit("map"), 0);
  
  // NOTE we could use an expression parser here to make this fancier
  // and check for the 'In -> Out' semicolon delimited syntax more
  // carefully, this isn't checking it very rigorously. But there are
  // no other cases we need to expect so far so being a bit sloppy
  // buys us a lot of simplicity.
  MD_Node *in_node = map_tag->first_child;
  MD_Node *arrow = in_node->next;
  MD_Node *out_node = arrow->next;
  {
   MD_Node *error_at = 0;
   if (MD_NodeIsNil(in_node))
   {
    error_at = map_tag;
   }
   else if (!MD_S8Match(arrow->string, MD_S8Lit("->"), 0) ||
            MD_NodeIsNil(out_node))
   {
    error_at = in_node;
   }
   if (error_at != 0)
   {
    MD_CodeLoc loc = MD_CodeLocFromNode(error_at);
    MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                    MD_S8Lit("a map's type should be specified like: `In -> Out`"));
   }
  }
  
  // construct type info for map
  GEN_TypedMapInfo *typed_map = 0;
  {
   // resolve in type info
   GEN_TypeInfo *in_type_info = gen_resolve_type_info_from_referencer(in_node);
   if (in_type_info != 0 &&
       in_type_info->kind != GEN_TypeKind_Enum)
   {
    MD_CodeLoc loc = MD_CodeLocFromNode(in_node);
    MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                    MD_S8Lit("a map's In type should be an enum"));
    in_type_info = 0;
   }
   
   // resolve out type info
   GEN_TypeInfo *out_type_info = gen_resolve_type_info_from_referencer(out_node);
   int out_is_type_info_ptr = 0;
   if (out_type_info == 0)
   {
    MD_String8 out_name = out_node->string;
    if (MD_S8Match(out_name, MD_S8Lit("$Type"), 0))
    {
     out_is_type_info_ptr = 1;
    }
    else
    {
     gen_type_resolve_error(out_node);
    }
   }
   
   // assemble typed map
   if (in_type_info != 0 && (out_type_info != 0 || out_is_type_info_ptr))
   {
    typed_map = MD_PushArray(arena, GEN_TypedMapInfo, 1);
    
    // fill primary values
    typed_map->in = in_type_info;
    typed_map->out = out_type_info;
    typed_map->out_is_type_info_ptr = out_is_type_info_ptr;
    
    // fill derived values
    typed_map->in_type_string  = in_type_info->node->string;
    if (out_is_type_info_ptr)
    {
     typed_map->out_type_string = MD_S8Lit("TypeInfo*");
    }
    else
    {
     typed_map->out_type_string = out_type_info->node->string;
    }
   }
  }
  
  // check for named children in the map tag
  int is_complete = MD_NodeHasChild(map_tag, MD_S8Lit("complete"), 0);
  MD_Node *default_val = gen_get_child_value(map_tag, MD_S8Lit("default"));
  MD_Node *auto_val = gen_get_child_value(map_tag, MD_S8Lit("auto"));
  
  // save to map
  map->typed_map = typed_map;
  map->is_complete = is_complete;
  map->default_val = default_val;
  map->auto_val = auto_val;
 }
}

void
gen_equip_map_cases(void)
{
 for (GEN_MapInfo *map = first_map;
      map != 0;
      map = map->next)
 {
  GEN_TypedMapInfo *typed_map = map->typed_map;
  if (typed_map != 0)
  {
   
   // get in type
   GEN_TypeInfo *in_type = typed_map->in;
   
   // build the list
   MD_b32 got_list = 1;
   GEN_MapCase *first_case = 0;
   GEN_MapCase *last_case = 0;
   int case_count = 0;
   
   MD_Node *map_root_node = map->node;
   
   for (MD_Node *case_node = map_root_node->first_child;
        !MD_NodeIsNil(case_node);
        case_node = MD_FirstNodeWithFlags(case_node->next, MD_NodeFlag_IsAfterComma))
   {
    // extract in & out
    MD_Node *in = case_node;
    MD_Node *arrow = in->next;
    MD_Node *out = arrow->next;
    if (!MD_S8Match(arrow->string, MD_S8Lit("->"), 0) ||
        MD_NodeIsNil(out))
    {
     MD_CodeLoc loc = MD_CodeLocFromNode(in);
     MD_PrintMessage(error_file, loc, MD_MessageKind_Error,
                     MD_S8Lit("a map's case should be specified like: `in -> out,`"));
     got_list = 0;
     goto skip_case;
    }
    
    // get enumerant from in_type
    GEN_TypeEnumerant *in_enumerant = gen_enumerant_from_name(in_type, in->string);
    if (in_enumerant == 0)
    {
     MD_CodeLoc loc = MD_CodeLocFromNode(in);
     MD_PrintMessageFmt(error_file, loc, MD_MessageKind_Error,
                        "'%.*s' is not a value in the enum '%.*s'",
                        MD_S8VArg(in->string), MD_S8VArg(in_type->node->string));
     got_list = 0;
     goto skip_case;
    }
    
    // save case
    if (got_list)
    {
     GEN_MapCase *map_case = MD_PushArray(arena, GEN_MapCase, 1);
     map_case->in_enumerant = in_enumerant;
     map_case->out = out;
     MD_QueuePush(first_case, last_case, map_case);
     case_count += 1;
    }
    
    skip_case:;
   }
   
   // save the list
   if (got_list)
   {
    map->first_case = first_case;
    map->last_case = last_case;
    map->case_count = case_count;
   }
  }
 }
}

void
gen_check_duplicate_cases(void)
{
 for (GEN_MapInfo *map = first_map;
      map != 0;
      map = map->next)
 {
  
  for (GEN_MapCase *node = map->first_case;
       node != 0;
       node = node->next)
  {
   GEN_TypeEnumerant *enumerant = node->in_enumerant;
   MD_String8 name = enumerant->node->string;
   for (GEN_MapCase *check = map->first_case;
        check != 0;
        check = check->next)
   {
    if (node == check)
    {
     break;
    }
    if (enumerant == check->in_enumerant)
    {
     MD_CodeLoc my_loc = MD_CodeLocFromNode(enumerant->node);
     MD_CodeLoc og_loc = MD_CodeLocFromNode(check->in_enumerant->node);
     MD_PrintMessageFmt(error_file, my_loc, MD_MessageKind_Error,
                        "'%.*s' is already defined", MD_S8VArg(name));
     MD_PrintMessageFmt(error_file, og_loc, MD_MessageKind_Note,
                        "see previous definition of '%.*s'", MD_S8VArg(name));
     break;
    }
    if (enumerant->value == check->in_enumerant->value)
    {
     MD_CodeLoc my_loc = MD_CodeLocFromNode(enumerant->node);
     MD_CodeLoc og_loc = MD_CodeLocFromNode(check->in_enumerant->node);
     MD_PrintMessageFmt(error_file, my_loc, MD_MessageKind_Error,
                        "'%.*s' has value %d which is already defined",
                        MD_S8VArg(name), enumerant->value);
     MD_PrintMessageFmt(error_file, og_loc, MD_MessageKind_Note,
                        "see previous definition '%.*s'",
                        MD_S8VArg(check->in_enumerant->node->string));
     break;
    }
   }
  }
  
 }
}

void
gen_check_complete_map_cases(void)
{
 for (GEN_MapInfo *map = first_map;
      map != 0;
      map = map->next)
 {
  GEN_TypedMapInfo *typed_map = map->typed_map;
  if (typed_map != 0 && map->is_complete)
  {
   int printed_message_for_this_map = 0;
   
   GEN_TypeInfo *in_type = typed_map->in;
   for (GEN_TypeEnumerant *enumerant = in_type->first_enumerant;
        enumerant != 0;
        enumerant = enumerant->next)
   {
    GEN_MapCase *existing_case = gen_map_case_from_enumerant(map, enumerant);
    
    if (existing_case == 0)
    {
     if (!printed_message_for_this_map)
     {
      printed_message_for_this_map = 1;
      MD_CodeLoc map_loc = MD_CodeLocFromNode(map->node); 
      MD_PrintMessage(error_file, map_loc, MD_MessageKind_Warning,
                      MD_S8Lit("map marked as complete is missing a case (or more)"));
     }
     MD_String8 enumerant_name = enumerant->node->string;
     MD_CodeLoc enumerant_loc = MD_CodeLocFromNode(enumerant->node);
     MD_PrintMessageFmt(error_file, enumerant_loc, MD_MessageKind_Note,
                        "see enumerant '%.*s'", MD_S8VArg(enumerant_name));
    }
   }
   
  }
 }
}

//~ generators ////////////////////////////////////////////////////////////////

// @notes Each generator function handles generating every instance of a
//  particular function. This means that there is only one place where the
//  generator gets called and only one place where the generated code gets
//  written. This keeps things simple but may be quite limiting depending on
//  the use case.
//
//  For instance, to have multiple groups of types and metadata generated to
//  different .h/.c file pairs, we could make the output file name a parameter
//  on the command line, and run the generator multiple times with different
//  metadesk files.
//
//  If we wanted to support references to entities accross files we would have
//  to handle all the files in one run of the generator, and the generator
//  functions would need to be called with different 'entities' for different
//  output files.

void
gen_type_definitions_from_types(FILE *out)
{
 // @notes This Metadesk helper generates a comment that points back here.
 //  Generating a comment like this can help a lot to with issues later.
 MD_PrintGenNoteCComment(out);
 
 MD_ArenaTemp scratch = MD_GetScratch(0, 0);
 
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  if (MD_NodeHasTag(type->node, MD_S8Lit("dont_output"), 0))
  {
   continue;
  }
  
  switch (type->kind)
  {
   default:break;
   
   case GEN_TypeKind_Struct:
   {
    MD_String8 struct_name = type->node->string;
    fprintf(out, "typedef struct %.*s %.*s;\n",
            MD_S8VArg(struct_name), MD_S8VArg(struct_name));
    fprintf(out, "struct %.*s\n", MD_S8VArg(struct_name));
    fprintf(out, "{\n");
    for (GEN_TypeMember *member = type->first_member;
         member != 0;
         member = member->next)
    {
     MD_String8 type_name = member->type->node->string;
     MD_String8 member_name = member->node->string;
     MD_Node *arrayTag = MD_TagFromString(member->node, MD_S8Lit("array"), 0);
     MD_Node *ptrTag = MD_TagFromString(member->node, MD_S8Lit("ptr"), 0);
     MD_u32  isPtrCount = 0;
     if (!MD_NodeIsNil(ptrTag))
     {
      isPtrCount = MD_CStyleIntFromString(ptrTag->first_child->string);
     }
     
     MD_String8List strList = {0};
     MD_S8ListPushFmt(scratch.arena, &strList, "%.*s ", MD_S8VArg(type_name));
     for (MD_u32 i = 0; i < isPtrCount; ++i)
     {
      MD_S8ListPush(scratch.arena, &strList, MD_S8Lit("*"));
     }
     MD_S8ListPushFmt(scratch.arena, &strList, "%.*s", MD_S8VArg(member_name));
     if (!MD_NodeIsNil(arrayTag))
     {
      MD_Node *tag_arg_node = arrayTag->first_child;
      MD_u32 arrayLen = MD_CStyleIntFromString(tag_arg_node->string);
      if (arrayLen != 0)
      {
       MD_S8ListPushFmt(scratch.arena, &strList, "[%d]", arrayLen);
      }
     }
     MD_S8ListPush(scratch.arena, &strList, MD_S8Lit(";\n"));
     
     {
      MD_String8 s = MD_S8ListJoin(scratch.arena, strList, 0);
      fprintf(out, "%.*s", MD_S8VArg(s));
     }
    }
    
    fprintf(out, "};\n\n");
   }break;
   
   case GEN_TypeKind_Enum:
   {
    MD_String8 enum_name = type->node->string;
    GEN_TypeInfo *underlying_type = type->underlying_type;
    
    // enum header
    if (underlying_type != 0)
    {
     MD_String8 underlying_type_name = underlying_type->node->string;
     fprintf(out, "typedef %.*s %.*s;\n",
             MD_S8VArg(underlying_type_name), MD_S8VArg(enum_name));
     fprintf(out, "enum\n");
    }
    else
    {
     fprintf(out, "typedef enum %.*s\n", MD_S8VArg(enum_name));
    }
    fprintf(out, "{\n");
    
    // enum body
    for (GEN_TypeEnumerant *enumerant = type->first_enumerant;
         enumerant != 0;
         enumerant = enumerant->next)
    {
     MD_String8 member_name = enumerant->node->string;
     fprintf(out, "%.*s_%.*s = %d,\n",
             MD_S8VArg(enum_name), MD_S8VArg(member_name), enumerant->value);
    }
    
    // enum footer
    if (underlying_type != 0)
    {
     fprintf(out, "};\n");
    }
    else
    {
     fprintf(out, "} %.*s;\n", MD_S8VArg(enum_name));
    }
    
   }break;
  }
 }
 
 MD_ReleaseScratch(scratch);
 
 fprintf(out, "\n");
}

void
gen_function_declarations_from_maps(FILE *out)
{
 MD_PrintGenNoteCComment(out);
 
 for (GEN_MapInfo *map = first_map;
      map != 0;
      map = map->next)
 {
  GEN_TypedMapInfo *typed_map = map->typed_map;
  if (typed_map != 0)
  {
   MD_String8 in_type  = typed_map->in_type_string;
   MD_String8 out_type = typed_map->out_type_string;
   
   fprintf(out, "%.*s %.*s(%.*s v);\n",
           MD_S8VArg(out_type), MD_S8VArg(map->node->string), MD_S8VArg(in_type));
  }
 }
 
 fprintf(out, "\n");
}

void
gen_type_info_declarations_from_types(FILE *out)
{
 MD_PrintGenNoteCComment(out);
 
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  MD_String8 name = type->node->string;
  fprintf(out, "extern TypeInfo %.*s_type_info;\n", MD_S8VArg(name));
 }
 
 fprintf(out, "\n");
}

void
gen_struct_member_tables_from_types(FILE *out)
{
 MD_PrintGenNoteCComment(out);
 
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  if (type->kind == GEN_TypeKind_Struct)
  {
   MD_String8 type_name = type->node->string;
   int member_count = type->member_count;
   
   fprintf(out, "TypeInfoMember %.*s_members[%d] = {\n", MD_S8VArg(type_name), member_count);
   
   for (GEN_TypeMember *member = type->first_member;
        member != 0;
        member = member->next)
   {
    MD_String8 member_name = member->node->string;
    MD_String8 member_type_name = member->type->node->string;
    int array_count_member_index = -1;
    if (member->array_count != 0)
    {
     array_count_member_index = member->array_count->member_index;
    }
    fprintf(out, "{\"%.*s\", %d, %d, &%.*s_type_info},\n",
            MD_S8VArg(member_name), (int)member_name.size,
            array_count_member_index, MD_S8VArg(member_type_name));
   }
   
   fprintf(out, "};\n");
  }
 }
 
 fprintf(out, "\n");
}

void
gen_enum_member_tables_from_types(FILE *out)
{
 MD_PrintGenNoteCComment(out);
 
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  if (type->kind == GEN_TypeKind_Enum)
  {
   MD_String8 type_name = type->node->string;
   int enumerant_count = type->enumerant_count;
   
   fprintf(out, "TypeInfoEnumerant %.*s_members[%d] = {\n",
           MD_S8VArg(type_name), enumerant_count);
   for (GEN_TypeEnumerant *enumerant = type->first_enumerant;
        enumerant != 0;
        enumerant = enumerant->next)
   {
    MD_String8 enumerant_name = enumerant->node->string;
    int value = enumerant->value;
    fprintf(out, "{\"%.*s\", %d, %d},\n",
            MD_S8VArg(enumerant_name), (int)enumerant_name.size,
            value);
   }
   
   fprintf(out, "};\n");
  }
 }
 
 fprintf(out, "\n");
}

void
gen_type_info_definitions_from_types(FILE *out)
{
 MD_ArenaTemp scratch = MD_GetScratch(0, 0);
 
 MD_PrintGenNoteCComment(out);
 
 for (GEN_TypeInfo *type = first_type;
      type != 0;
      type = type->next)
 {
  MD_String8 type_name = type->node->string;
  
  switch (type->kind)
  {
   default:break;
   
   case GEN_TypeKind_Basic:
   {
    int size = type->size;
    fprintf(out, "TypeInfo %.*s_type_info = "
            "{TypeKind_Basic, \"%.*s\", %d, %d, 0, 0};\n",
            MD_S8VArg(type_name),
            MD_S8VArg(type_name), (int)type_name.size, size);
   }break;
   
   case GEN_TypeKind_Struct:
   {
    int child_count = type->member_count;
    fprintf(out, "TypeInfo %.*s_type_info = "
            "{TypeKind_Struct, \"%.*s\", %d, %d, %.*s_members, 0};\n",
            MD_S8VArg(type_name),
            MD_S8VArg(type_name), (int)type_name.size, child_count, MD_S8VArg(type_name));
   }break;
   
   case GEN_TypeKind_Enum:
   {
    MD_String8 underlying_type_ptr_expression = MD_S8Lit("0");
    if (type->underlying_type != 0)
    {
     MD_String8 underlying_type_name = type->underlying_type->node->string;
     underlying_type_ptr_expression = MD_S8Fmt(scratch.arena, "&%.*s_type_info",
                                               MD_S8VArg(underlying_type_name));
    }
    
    int child_count = type->enumerant_count;
    fprintf(out, "TypeInfo %.*s_type_info = "
            "{TypeKind_Enum, \"%.*s\", %d, %d, %.*s_members, %.*s};\n",
            MD_S8VArg(type_name),
            MD_S8VArg(type_name), (int)type_name.size, child_count, MD_S8VArg(type_name),
            MD_S8VArg(underlying_type_ptr_expression));
   }break;
  }
 }
 
 fprintf(out, "\n");
 
 MD_ReleaseScratch(scratch);
}

void
gen_function_definitions_from_maps(FILE *out)
{
 MD_ArenaTemp scratch = MD_GetScratch(0, 0);
 
 MD_PrintGenNoteCComment(out);
 
 for (GEN_MapInfo *map = first_map;
      map != 0;
      map = map->next)
 {
  GEN_TypedMapInfo *typed_map = map->typed_map;
  if (typed_map != 0)
  {
   MD_String8 in_type  = typed_map->in_type_string;
   MD_String8 out_type = typed_map->out_type_string;
   
   fprintf(out, "%.*s\n", MD_S8VArg(out_type));
   fprintf(out, "%.*s(%.*s v)\n", MD_S8VArg(map->node->string), MD_S8VArg(in_type));
   fprintf(out, "{\n");
   fprintf(out, "%.*s result;\n", MD_S8VArg(out_type));
   fprintf(out, "switch (v)\n");
   fprintf(out, "{\n");
   
   // default
   fprintf(out, "default:\n");
   fprintf(out, "{\n");
   if (!MD_NodeIsNil(map->default_val))
   {
    MD_String8 default_expr = map->default_val->string;
    MD_String8 val_expr = default_expr;
    if (typed_map->out_is_type_info_ptr)
    {
     val_expr = MD_S8Fmt(scratch.arena, "&%.*s_type_info", MD_S8VArg(default_expr));
    }
    fprintf(out, "result = %.*s;\n", MD_S8VArg(val_expr));
   }
   else if (typed_map->out_is_type_info_ptr)
   {
    fprintf(out, "result = 0;\n");
   }
   fprintf(out, "}break;\n");
   
   // auto cases
   if (!MD_NodeIsNil(map->auto_val))
   {
    int map_has_an_implicit_case = 0;
    for (GEN_TypeEnumerant *enumerant = typed_map->in->first_enumerant;
         enumerant != 0;
         enumerant = enumerant->next)
    {
     GEN_MapCase *explicit_case = gen_map_case_from_enumerant(map, enumerant);
     
     if (explicit_case == 0)
     {
      map_has_an_implicit_case = 1;
      MD_String8 in_expr = enumerant->node->string;
      fprintf(out, "case %.*s_%.*s:\n", MD_S8VArg(in_type), MD_S8VArg(in_expr));
     }
    }
    
    if (map_has_an_implicit_case)
    {
     MD_String8 auto_expr = map->auto_val->string;
     MD_String8 val_expr = auto_expr;
     if (typed_map->out_is_type_info_ptr)
     {
      val_expr = MD_S8Fmt(scratch.arena, "&%.*s_type_info", MD_S8VArg(auto_expr));
     }
     
     fprintf(out, "{\n");
     fprintf(out, "result = %.*s;\n", MD_S8VArg(val_expr));
     fprintf(out, "}break;\n");
    }
   }
   
   // explicit cases
   for (GEN_MapCase *map_case = map->first_case;
        map_case != 0;
        map_case = map_case->next)
   {
    MD_String8 in_expr = map_case->in_enumerant->node->string;
    MD_String8 out_expr = map_case->out->string;
    MD_String8 val_expr = out_expr;
    if (typed_map->out_is_type_info_ptr)
    {
     val_expr = MD_S8Fmt(scratch.arena, "&%.*s_type_info", MD_S8VArg(out_expr));
    }
    
    fprintf(out, "case %.*s_%.*s:\n", MD_S8VArg(in_type), MD_S8VArg(in_expr));
    fprintf(out, "{\n");
    fprintf(out, "result = %.*s;\n", MD_S8VArg(val_expr));
    fprintf(out, "}break;\n");
   }
   
   fprintf(out, "}\n");
   fprintf(out, "return(result);\n");
   fprintf(out, "}\n");
  }
 }
 
 fprintf(out, "\n");
 
 MD_ReleaseScratch(scratch);
}


