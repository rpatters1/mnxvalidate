## Validation ideas

### 3.0 completed

- detect if a beam contains a mix of grace notes and non grace notes.
- detect if different beams in the same measure contain the same event at the same level.
- detect if a beam contains no events.
- detect if beam events are not in measure/time order, including when the first event is not in the beam's containing measure.
- detect if a beam includes an event that is actually a multi-note tremolo.
- detect if beam events are from different voices.
- detect if a beam hook direction is specified for more than one event.
- detect if a sequence contains more musical time than the time signature specifies.
- detect if tuplet and multi-note tremolo contents do not add up to the required exact duration.
- detect tuplets with zero inner duration.
- detect multi-note tremolos with fewer than two events.
- detect if a content array in a sequence contains invalid entities.
- validate tie semantics for `target`, `lv`, and `targetType` combinations.
- validate that tied kit notes target the same kit component.
- validate note pitch alteration range (+/-3).
- validate that kit-note components exist in the part's kit.
- validate global sound midi-number range (0-127).
- detect if an ottava ends before it begins.
- detect parts with zero staves.
- validate that part kit sounds reference ids defined in `global.sounds`.
- require beginning clefs for each non-kit staff in the first measure.
- detect clef and sequence staff references to non-existent part staves.
- detect full-measure sequences that still contain content.
- detect empty layout groups.
- detect layout staff sources with empty or invalid part references.
- detect layout staff sources with out-of-range staff numbers for their part.
- detect duplicate part+voice assignments within a layout staff.
- validate page/system/layout-change layout references.
- detect systems that do not start on the first measure and systems that regress/overlap previous systems.
- detect layout changes that start at or past end-of-measure.
- validate voice uniqueness for sequence objects per part-measure.

### 2.0 completed

- confirm staff systems neither skip nor overlap measures.
- confirm that global.lyrics.lineMetadata and global.lyrics.lineOrder arrays match if both are provided.
- confirm event ids and note ids are unique
- confirm that event objects either have a duration or a full-measure indication but not both.
- confirm that events are either rests or have notes but not both.
- confirm that lyric line labels in event objects are valid values, if global.lyrics provided the valid values.
- confirm tied-to note ids exist and have the same part, pitch name, octave, and alteration as their counterparts.
- confirm events in beams are only specified once.
- confirm beam nesting level does not exceed number of flags on note values of enclosed events.
- confirm that the number of flags on events specified with beam hooks exceeds the beam nesting level.
- validate slurs (target, startNote, endNote)

### 1.0 completed

- confirm multimeasure rests span measures that exist
- confirm measure counts in each part (optionally: it may be permissible to have different numbers of measures per part. need to check)
- confirm layouts in "scores" have been defined in "layouts". This is required by the MNX doc and not enforced by the schema.
- confirm measures exist in staff systems
- confirm "part" and "labelref" values in staff and staff source objects contain valid values/part references (part ids)
- valid values for enums (some of these are enforced by the schema; we'll need to check):
    - "labelref" (not enforced)
    - "stem" (staff source stem direction) -- enforced by schema
    - "symbol" (group bracket) -- enforced by schema

### Questions and Suggestions

- more symbol styles for groups (in particular, deskBracket)
- since the staff object has a "labelref" property, should it not also have a "part" property?
- staff group objects need a labelref alternative (and "part" that it references).
- labelRef values are not enforced by the schema
- there needs to be a mechanism to specify automatic name/shortName for "labelref" values.
- should "systems" be a required propery of the page object?
- handle "color"
